// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter.h"
#include "EnhancedInputComponent.h"	   // UEnhancedInputComponent
#include "EnhancedInputSubsystems.h"   // UEnhancedInputLocalPlayerSubsystem
#include "InputActionValue.h"		   // FInputActionValue
#include "Components/InputComponent.h" // UInputComponent base

#include "GameFramework/Controller.h" // APlayerController
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h" // USpringArmComponent

#include "EngineUtils.h"

#include "Camera/PlayerCameraManager.h"
#include "Camera/CameraComponent.h" // UCameraComponent
#include "DrawDebugHelpers.h"

#include "Kismet/KismetMathLibrary.h"
#include "Math/UnrealMathUtility.h" // FMath

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		if (UEnhancedInputLocalPlayerSubsystem *Subsystem =
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(MovingContext, 0);
			UE_LOG(LogTemp, Display, TEXT("MappingContext registered in BeginPlay"));
		}
	}

	if (PlayerController && FocusIndicatorWidgetClass)
	{
		UE_LOG(LogTemp, Display, TEXT("FocusIndicatorWidgetClass registered in BeginPlay"));
		FocusIndicatorWidget = CreateWidget<UUserWidget>(
			PlayerController,		  // 소유자
			FocusIndicatorWidgetClass // 에디터에서 지정한 위젯 클래스
		);
		if (FocusIndicatorWidget)
		{
			FocusIndicatorWidget->AddToViewport();						   // 화면에 추가
			FocusIndicatorWidget->SetVisibility(ESlateVisibility::Hidden); // 초기엔 숨김
			UE_LOG(LogTemp, Display, TEXT("FocusIndicatorWidget registered in BeginPlay"));
		}
	}

	CameraBoom = FindComponentByClass<USpringArmComponent>(); // 카메라 스프링 암 컴포넌트 찾기
	FollowCamera = FindComponentByClass<UCameraComponent>();  // 카메라 컴포넌트 찾기

	if (UAnimInstance *AnimInst = GetMesh()->GetAnimInstance())
	{
		AnimInst->OnPlayMontageNotifyBegin.AddDynamic(this, &APlayerCharacter::OnNotifyBegin); // NotifyState 시작(NotifyBegin) 바인딩
		AnimInst->OnPlayMontageNotifyEnd.AddDynamic(this, &APlayerCharacter::OnNotifyEnd);	   // NotifyState 종료(NotifyEnd) 바인딩
	}
	MoveComp = GetCharacterMovement();
	if (MoveComp)
	{
		UE_LOG(LogTemp, Display, TEXT("Movement Component Is Successfully Set"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed To Set Movement Component"));
	}

	bUseControllerRotationYaw = false; // 컨트롤러 회전(Yaw) 사용하지 않음

	if (MoveComp)
	{
		MoveComp->bOrientRotationToMovement = true; // 이동 방향으로 자동 회전
		MoveComp->RotationRate = FRotator(0.f, 500.0f, 0.f);
	}

	SwordMeshComponent = FindComponentByClass<UStaticMeshComponent>();

	if (SwordMeshComponent)
	{
		UE_LOG(LogTemp, Display, TEXT("Sword Mesh Component Is Successfully Set : %s"), *SwordMeshComponent->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed To Set Sword Mesh Component"));
	}
}
// Move 입력 시 호출되는 함수
void APlayerCharacter::Move(const FInputActionValue &Value)
{
	if (bIsAttacking || MoveComp->IsFalling())
	{
		return;
	}
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (PlayerController && (MovementVector.X != 0.f || MovementVector.Y != 0.f))
	{
		const FRotator YawRotation(0, PlayerController->GetControlRotation().Yaw, 0);
		const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDir, MovementVector.Y);
		AddMovementInput(RightDir, MovementVector.X);
		MovementState = EMovementState::MS_Moving;
	}
}

// Look 입력 시 호출되는 함수
void APlayerCharacter::Look(const FInputActionValue &Value)
{
	if (!bIsFocusing)
	{
		FVector2D LookVector = Value.Get<FVector2D>();
		AddControllerYawInput(LookVector.X);
		AddControllerPitchInput(-LookVector.Y);
	}
}

bool APlayerCharacter::SearchFocusTarget()
{
	FocusTargetArray.Empty();

	if (!PlayerController)
	{
		return false;
	}

	// 1) 카메라 위치·방향
	FVector CamLoc;
	FRotator CamRot;
	PlayerController->GetPlayerViewPoint(CamLoc, CamRot); // 플레이어의 카메라 위치/회전 정보를 가져옴
	FVector CamForward = CamRot.Vector();

	// 2) Sweep 파라미터
	FVector BoxHalfExtents = FVector(500.f, 500.f, 500.f);

	const float SweepDistance = FocusSearchRadius;
	FVector SweepStart = CamLoc + CamForward * 800;
	FVector SweepEnd = CamLoc + CamForward * SweepDistance;

	// 3) SweepMultiByObjectType 호출
	FCollisionShape BoxShape = FCollisionShape::MakeBox(BoxHalfExtents);
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	TArray<FHitResult> HitResults;
	FCollisionQueryParams Params(NAME_None, false, this);

	bool bHitAny = GetWorld()->SweepMultiByObjectType(
		HitResults,
		SweepStart,
		SweepEnd,
		FQuat::Identity,
		ObjectParams,
		BoxShape,
		Params);

	// 4) 디버그: 스윕 시작/끝 박스 + 연결선
	DrawDebugBox(GetWorld(), SweepStart, BoxHalfExtents, FQuat::Identity, FColor::Red, false, 1.0f, 0, 2.0f);
	DrawDebugBox(GetWorld(), SweepEnd, BoxHalfExtents, FQuat::Identity, FColor::Cyan, false, 1.0f, 0, 2.0f);
	DrawDebugLine(GetWorld(), SweepStart, SweepEnd, FColor::Green, false, 1.0f, 0, 2.0f);

	if (!bHitAny)
	{
		return false;
	}

	// 5) 히트 결과 순회하며 Pawn만 배열에 추가
	for (const FHitResult &HR : HitResults)
	{
		if (APawn *P = Cast<APawn>(HR.GetActor()))
		{
			int32 PastSz = FocusTargetArray.Num();
			FocusTargetArray.AddUnique(P);
			if (PastSz < FocusTargetArray.Num())
				UE_LOG(LogTemp, Display, TEXT("Sweep hit pawn: %s"), *P->GetName());
		}
	}

	if (FocusTargetArray.Num() == 0)
	{
		return false;
	}

	CurrentFocusIndex = 0;

	return true;
}

void APlayerCharacter::UpdateFocusCamera(float DeltaTime)
{
	if (FocusTargetArray.Num() == 0 ||
		CurrentFocusIndex < 0 ||
		!FocusTargetArray.IsValidIndex(CurrentFocusIndex))
	{
		UE_LOG(LogTemp, Error, TEXT("Focus Target Array Is Invalid!"));
		return;
	}

	if (!FocusIndicatorWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed To Load Focus Indicator Widget!"));
		return;
	}

	// 1) 타겟 위치 구하기
	APawn *Target = FocusTargetArray[CurrentFocusIndex];
	FVector TargetLoc = Target->GetActorLocation();
	FVector PlayerLoc = this->GetActorLocation();
	if (FVector::Dist(TargetLoc, PlayerLoc) > FocusSearchRadius)
	{
		ToogleFocus();
		return;
	}

	FVector2D ScreenPos;
	if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PlayerController, TargetLoc, ScreenPos, false))
	{
		FocusIndicatorWidget->SetPositionInViewport(ScreenPos + FVector2D(0.0f, -15.0f), false);
	}

	// 2) 카메라(또는 스프링암) 월드 위치
	FVector CamLoc = FollowCamera
						 ? FollowCamera->GetComponentLocation()
						 : CameraBoom->GetComponentLocation();

	// 3) 바라볼 회전 계산
	FRotator DesiredRot = UKismetMathLibrary::FindLookAtRotation(CamLoc, TargetLoc);

	// 4) 부드러운 보간
	FRotator CurrentRot = PlayerController->GetControlRotation();
	FRotator NewRot = FMath::RInterpTo(CurrentRot, DesiredRot, DeltaTime, 10.f);

	// 5) 컨트롤러 회전에 설정 → 카메라가 따라감
	PlayerController->SetControlRotation(NewRot);
}

void APlayerCharacter::SwitchFocus(const FInputActionValue &Value)
{
	ToogleFocus();
}
void APlayerCharacter::ToogleFocus()
{
	if (bIsFocusing)
	{
		bIsFocusing = false;
		FocusIndicatorWidget->SetVisibility(ESlateVisibility::Hidden);

		bUseControllerRotationYaw = false; // 컨트롤러 회전 사용하지 않음
		if (MoveComp)
		{
			MoveComp->bOrientRotationToMovement = true; // 이동 방향으로 자동 회전
		}
	}
	else if (SearchFocusTarget())
	{
		bIsFocusing = true;
		StopRunning();
		FocusIndicatorWidget->SetVisibility(ESlateVisibility::Visible);

		bUseControllerRotationYaw = true; // 컨트롤러 회전 사용
		if (MoveComp)
		{
			MoveComp->bOrientRotationToMovement = false; // 이동 방향으로 자동 회전하지 않음
		}
	}

	UE_LOG(LogTemp, Display, TEXT("%s"),
		   bIsFocusing
			   ? TEXT("Now focusing is on")
			   : TEXT("Focusing is off"));
}

void APlayerCharacter::ChangeFocusTarget(const FInputActionValue &Value)
{
	float Scroll = Value.Get<float>();
	UE_LOG(LogTemp, Display, TEXT("%f"), Scroll);
	CurrentFocusIndex += Scroll;
	if (CurrentFocusIndex < 0) // 가장 가까운 타겟에서 가장 먼 타겟으로 변경한 경우
	{
		SearchFocusTarget();
		CurrentFocusIndex = FocusTargetArray.Num() - 1;
	}
	else if (CurrentFocusIndex >= FocusTargetArray.Num()) // 가장 먼 타겟에서 가장 가까운 타겟으로 변경한 경우
	{
		SearchFocusTarget();
	}
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bIsFocusing)
	{
		UpdateFocusCamera(DeltaTime);
	}
	if (MoveComp->IsFalling())
	{
		MovementState = EMovementState::MS_Falling;
	}
	if (bIsAttacking)
	{
		DoAttackTrace();
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent *EnhancedInput =
			Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
		EnhancedInput->BindAction(SwitchFocusAction, ETriggerEvent::Started, this, &APlayerCharacter::SwitchFocus);
		EnhancedInput->BindAction(ChangeFocusTargetAction, ETriggerEvent::Started, this, &APlayerCharacter::ChangeFocusTarget);
		EnhancedInput->BindAction(LightAttackAction, ETriggerEvent::Started, this, &APlayerCharacter::LightAttack);

		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		EnhancedInput->BindAction(RunDodgeAction, ETriggerEvent::Started, this, &APlayerCharacter::RunDodge);
		EnhancedInput->BindAction(RunDodgeAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopRunning);

		EnhancedInput->BindAction(GuardAction, ETriggerEvent::Started, this, &APlayerCharacter::Guard);
		EnhancedInput->BindAction(GuardAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopGuarding);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to bind MoveAction!"));
	}
}

void APlayerCharacter::LightAttack(const FInputActionValue &Value)
{
	// 첫 공격 시작
	if (!bIsAttacking && CheckCanMove())
	{
		bIsAttacking = true;
		CurrentAttackCombo = 1;
		PlayLightAttackMontage();
		return;
	}

	// 공격 중이고 윈도우 안이라면 다음 콤보 요청
	if (bCanCombo)
	{
		bWantCombo = true;
	}
}
void APlayerCharacter::PlayLightAttackMontage()
{
	UAnimMontage *CurrentAnimMontage;
	if (bIsRunning)
	{
		CurrentAnimMontage = RunAttackMontage;
		CurrentAttackCombo = 1;
	}
	else
	{
		CurrentAnimMontage = AttackMontages[CurrentAttackCombo - 1];
	}
	if (CurrentAnimMontage)
	{
		MovementState = EMovementState::MS_Attacking;
		PlayAnimMontage(CurrentAnimMontage, 1.0f);
		UE_LOG(LogTemp, Log, TEXT("Combo Stack : %d/%d"), CurrentAttackCombo, AttackMontages.Num());
	}
}

void APlayerCharacter::PlayHitMontage()
{
	int Index = FMath::RandRange(0, HitMontages.Num() - 1);
	UAnimMontage *CurrentAnimMontage = HitMontages[Index];
	bIsHit = true;
	if (CurrentAnimMontage)
	{
		// MovementState = EMovementState::MS_Hit;
		PlayAnimMontage(CurrentAnimMontage, 1.0f);
		UE_LOG(LogTemp, Log, TEXT("Hit Montage[%d] Played"), Index);
	}
}

void APlayerCharacter::OnNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload &Payload)
{

	// NotifyState 이름이 ComboWindow 일 때만 반응
	if (NotifyName == TEXT("ComboWindow"))
	{
		bCanCombo = true;
		// UE_LOG(LogTemp, Log, TEXT("Combo Window Open"));
	}
	if (NotifyName == TEXT("EndDodge"))
	{
		MovementState = EMovementState::MS_Idle;
	}
}

void APlayerCharacter::OnNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload &Payload)
{
	if (NotifyName == TEXT("ComboWindow"))
	{
		bCanCombo = false;
		if (bWantCombo)
		{
			bWantCombo = false;
			CurrentAttackCombo = (CurrentAttackCombo + 1) % (AttackMontages.Num() + 1);
			if (!CurrentAttackCombo)
			{
				CurrentAttackCombo++;
			}
			PlayLightAttackMontage();
		}
		else
		{
			CurrentAttackCombo = 0;
			bIsAttacking = false;
			MovementState = EMovementState::MS_Idle;
		}
	}
}

void APlayerCharacter::Landed(const FHitResult &Hit)
{
	Super::Landed(Hit);
	bJustLanded = true;
	MovementState = EMovementState::MS_Idle;
}

void APlayerCharacter::Jump()
{
	if (bIsAttacking) // 공격 중 Jump 방지
	{
		return;
	}
	Super::Jump();
}

void APlayerCharacter::RunDodge(const FInputActionValue &Value)
{
	if (!bIsFocusing)
	{
		Run();
	}
	else if (CheckCanMove())
	{
		Dodge();
	}
}

void APlayerCharacter::Run()
{
	if (!bIsRunning)
	{
		UE_LOG(LogTemp, Display, TEXT("Run!"));
		bIsRunning = true;
		MoveComp->MaxWalkSpeed = 1100;
	}
}

void APlayerCharacter::StopRunning()
{
	if (bIsRunning)
	{
		UE_LOG(LogTemp, Display, TEXT("Walk!"));
		bIsRunning = false;
		MoveComp->MaxWalkSpeed = 550;
	}
}

void APlayerCharacter::Dodge()
{
	MovementState = EMovementState::MS_Dodging;
	PlayAnimMontage(DodgeMontage, 1.5f);
}

bool APlayerCharacter::CheckCanMove()
{
	return (MovementState == EMovementState::MS_Moving || MovementState == EMovementState::MS_Idle);
}

void APlayerCharacter::Guard()
{
	if (!bIsGuarding)
	{
		bIsGuarding = true;
	}
}

void APlayerCharacter::StopGuarding()
{
	if (bIsGuarding)
	{
		bIsGuarding = false;
	}
}
void APlayerCharacter::DoAttackTrace()
{
	if (SwordMeshComponent) // SwordMeshComponent가 valid한지 확인
	{
		UWorld *World = GetWorld();
		if (World)
		{
			FVector Start = SwordMeshComponent->GetComponentLocation();
			FVector Forward = SwordMeshComponent->GetRightVector();
			FVector End = Start + Forward * LightAttackRange;

			// 쿼리 파라미터 (자기 자신 무시)
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(this);
			QueryParams.bTraceComplex = true;

			// Pawn 오브젝트 타입만 검사하도록 설정
			FCollisionObjectQueryParams ObjectQueryParams;
			ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
			TArray<FHitResult> Hits;
			// FHitResult Hit;
			bool bHit = World->LineTraceMultiByObjectType(Hits, Start, End, ObjectQueryParams, QueryParams);
			if (bHit)
			{
				for (auto Hit : Hits)
				{
					APlayerCharacter *HitPlayerCharacter = nullptr;
					if (bHit)
					{
						HitPlayerCharacter = Cast<APlayerCharacter>(Hit.GetActor());
					}
					if (bHit && HitPlayerCharacter)
					{
						UE_LOG(LogTemp, Log, TEXT("[AttackTrace] Hit Pawn Actor: %s"),
							   *Hit.GetActor()->GetName());
						HitPlayerCharacter->PlayHitMontage();
						// 공격자(this) 위치에서 피격자 위치로의 벡터 (피격자가 공격자를 바라보려면 공격자 - 피격자)
						FVector ToAttacker = GetActorLocation() - HitPlayerCharacter->GetActorLocation();
						ToAttacker.Z = 0.f;

						if (!ToAttacker.IsNearlyZero())
						{
							FRotator LookAtRot = ToAttacker.Rotation();
							LookAtRot.Pitch = 0.f;
							LookAtRot.Roll = 0.f;

							// 가능하면 컨트롤러 회전도 설정 (애니메이션/컨트롤러에 따라 필요)
							if (AController *HitCtrl = HitPlayerCharacter->GetController())
							{
								HitCtrl->SetControlRotation(LookAtRot);
							}

							// 액터 자체 회전도 설정
							HitPlayerCharacter->SetActorRotation(LookAtRot);
							// HitPlayerCharacter->GetCharacterMovement()->AddImpulse(FVector(100.0f, 0, 0), false);
						}
						bIsHit = true;
					}
				}
				// DrawDebugLine(World, Start, End, bHit ? FColor::Red : FColor::Green, false, 1.0f, 0, 1.0f);}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[AttackTrace] GetWorld() returned null"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[AttackTrace] SwordMeshComponent is null"));
	}
}