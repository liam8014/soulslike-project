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

#include "TimerManager.h"

#include "Kismet/GameplayStatics.h" // UGameplayStatics::SetGlobalTimeDilation
#include "Engine/World.h"			// UWorld
#include "Containers/Ticker.h"		// FTSTicker, FTickerDelegate
#include "../UI/StatusBar.h"

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
			UE_LOG(LogTemp, Display, TEXT("MappingContext regFistered in BeginPlay"));
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
		MoveComp->MaxWalkSpeed = WalkSpeed;
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

	if (StatusBarClass)
	{
		StatusBar = CreateWidget<UStatusBar>(GetWorld(), StatusBarClass);
		if (StatusBar)
		{
			StatusBar->AddToViewport();
			AddHealth(0.0f);
			AddStamina(0.0f);
		}
	}
}
// Move 입력 시 호출되는 함수
void APlayerCharacter::Move(const FInputActionValue &Value)
{
	if (!CanMove())
	{
		return;
	}
	bIsAttacking = false; // 공격 상태 해제

	FVector2D Input = Value.Get<FVector2D>(); // X=Right, Y=Forward
	if (Input.X == 0.f && Input.Y == 0.f)
		return; // 입력 없음

	const FRotator Yaw(0, PlayerController->GetControlRotation().Yaw, 0); // 카메라 yaw
	const FVector CamF = FRotationMatrix(Yaw).GetUnitAxis(EAxis::X);	  // 카메라 전방
	const FVector CamR = FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y);	  // 카메라 우측

	{
		FVector WorldInput = CamF * Input.Y + CamR * Input.X; // 월드 입력 벡터
		float InLen = WorldInput.Size();					  // 입력 길이
		if (InLen <= KINDA_SMALL_NUMBER)
			return; // 안전 체크

		FVector MoveDir = WorldInput / InLen; // 이동 방향(단위벡터)

		FVector Facing = GetActorForwardVector();
		Facing.Z = 0.f;
		Facing = Facing.GetSafeNormal(); // 플레이어 바라보는 방향
		if (Facing.IsNearlyZero())
		{
			AddMovementInput(MoveDir, InLen);
			ChangeMovement(EMovementState::MS_Moving);
			return;
		} // Facing 이상시 기본 처리

		float Dot = FVector::DotProduct(MoveDir, Facing); // 정렬도 (-1..1)
		if (Dot >= AlignmentThreshold)
		{
			AddMovementInput(MoveDir, InLen);
			ChangeMovement(EMovementState::MS_Moving);
			return;
		} // 거의 일치하면 감속 없음

		FVector Right = FVector::CrossProduct(FVector::UpVector, Facing).GetSafeNormal(); // 플레이어 우측 벡터
		float ForwardComp = FVector::DotProduct(MoveDir, Facing);						  // 정면 컴포넌트
		float RightComp = FVector::DotProduct(MoveDir, Right);							  // 측면 컴포넌트

		float AdjForward = (ForwardComp > 0.f) ? ForwardComp : (ForwardComp * BackwardMovementMultiplier); // 전/후 보정
		float AdjRight = RightComp * SideMovementMultiplier;											   // 좌우 보정

		FVector AdjDir = Facing * AdjForward + Right * AdjRight; // 보정된 방향 벡터
		float Scale = InLen * AdjDir.Size();					 // 최종 스케일
		if (Scale > KINDA_SMALL_NUMBER)
		{
			AddMovementInput(AdjDir.GetSafeNormal(), Scale); // 입력 적용
			ChangeMovement(EMovementState::MS_Moving);
		}
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

	// 카메라 위치·방향
	FVector CamLoc;
	FRotator CamRot;
	PlayerController->GetPlayerViewPoint(CamLoc, CamRot); // 플레이어의 카메라 위치/회전 정보를 가져옴
	FVector CamForward = CamRot.Vector();

	// Sweep 파라미터
	FVector BoxHalfExtents = FVector(300.f, 500.f, 500.f);

	const float SweepDistance = FocusSearchRadius;
	FVector SweepStart = CamLoc + CamForward * 800;
	FVector SweepEnd = CamLoc + CamForward * SweepDistance;

	// SweepMultiByObjectType 호출
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

	// 디버그: 스윕 시작/끝 박스 + 연결선
	// DrawDebugBox(GetWorld(), SweepStart, BoxHalfExtents, FQuat::Identity, FColor::Red, false, 1.0f, 0, 2.0f);
	// DrawDebugBox(GetWorld(), SweepEnd, BoxHalfExtents, FQuat::Identity, FColor::Cyan, false, 1.0f, 0, 2.0f);
	// DrawDebugLine(GetWorld(), SweepStart, SweepEnd, FColor::Green, false, 1.0f, 0, 5.0f);

	if (!bHitAny)
	{
		return false;
	}

	// 히트 결과 순회하며 Pawn만 배열에 추가
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
		ToogleFocus();
		return;
	}

	if (!FocusIndicatorWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed To Load Focus Indicator Widget!"));
		return;
	}

	// 타겟 위치 구하기
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

	// 카메라(또는 스프링암) 월드 위치
	FVector CamLoc = FollowCamera
						 ? FollowCamera->GetComponentLocation()
						 : CameraBoom->GetComponentLocation();

	// 바라볼 회전 계산
	FRotator DesiredRot = UKismetMathLibrary::FindLookAtRotation(CamLoc, TargetLoc);
	if (DesiredRot.Pitch < -20) // 최소 Pitch 지정
	{
		DesiredRot.Pitch = -20;
	}

	// 부드러운 보간
	FRotator CurrentRot = PlayerController->GetControlRotation();
	FRotator NewRot = FMath::RInterpTo(CurrentRot, DesiredRot, DeltaTime, 10.f);

	// 컨트롤러 회전에 설정 -> 카메라가 따라감
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
	if (!bIsFocusing)
	{
		return;
	}
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
		ChangeMovement(EMovementState::MS_Falling);
	}
	if (bIsAttacking && bCanCombo)
	{
		DoAttackTrace();
	}

	if (MovementState == EMovementState::MS_Moving && GetVelocity().Size() == 0)
	{
		ChangeMovement(EMovementState::MS_Idle);
	}

	if (bIsRunning)
	{
		AddStamina(-StaminaRunCost);
	}
	else
	{

		if ((MovementState == EMovementState::MS_Idle || MovementState == EMovementState::MS_Moving) && Stamina < MaxStamina)
		{
			AddStamina(bIsGuarding ? StaminaRegenAmount * 0.5 : StaminaRegenAmount);
		}
	}
	// UpdateUI();

	if (CanMove() && bAutoAttack && !bIsAttacking)
	{
		LightAttack();
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
		EnhancedInput->BindAction(AttackAction, ETriggerEvent::Started, this, &APlayerCharacter::Attack);

		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		EnhancedInput->BindAction(RunAction, ETriggerEvent::Started, this, &APlayerCharacter::Run);
		EnhancedInput->BindAction(RunAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopRunning);

		EnhancedInput->BindAction(DodgeAction, ETriggerEvent::Started, this, &APlayerCharacter::Dodge);

		EnhancedInput->BindAction(GuardAction, ETriggerEvent::Started, this, &APlayerCharacter::Guard);
		EnhancedInput->BindAction(GuardAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopGuarding);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to bind MoveAction!"));
	}
}

void APlayerCharacter::OnNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload &Payload)
{

	// NotifyState 이름이 ComboWindow 일 때만 반응
	if (NotifyName == TEXT("ComboWindow"))
	{
		bCanCombo = true;
	}
	if (NotifyName == TEXT("EndDodge"))
	{
		ChangeMovement(EMovementState::MS_Idle);
		bCanAttack = true;
		bIsDodging = false;
	}
	if (NotifyName == TEXT("EndHit"))
	{
		// bIsHit = false;
		// bIsAttacking = false;
		// bIsDodging = false;
		// bCanAttack = true;
		bCanAttack = true;
		ResetMovement();
	}
	if (NotifyName == TEXT("EndBlockHit"))
	{
		UE_LOG(LogTemp, Display, TEXT("End Block Hit"));
		ResetMovement();
	}
	if (NotifyName == TEXT("EndParry"))
	{
		bIsParrying = false;
		ResetMovement();
	}
	if (NotifyName == TEXT("EndStun"))
	{
		ResetMovement();
	}
}

void APlayerCharacter::OnNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload &Payload)
{
	if (NotifyName == TEXT("ComboWindow"))
	{
		bCanCombo = false;
		if (bAutoAttack) // 자동 공격 더미용 스태미나 회복
		{
			AddStamina(StaminaLightAttackCost);
		}
		if (bWantCombo || bAutoAttack && bCanAttack)
		{
			bWantCombo = false;
			CurrentAttackCombo = (CurrentAttackCombo + 1) % (AttackMontages.Num() + 1);
			if (!CurrentAttackCombo) // 현재 공격 콤보 0 방지
			{
				CurrentAttackCombo++;
			}
			PlayLightAttackMontage();
		}
		else if (bIsAttacking)
		{
			CurrentAttackCombo = 0;
			bIsAttacking = false;
			ResetMovement();
		}
	}
}
void APlayerCharacter::Attack()
{
	if (bIsGuarding)
	{
		Parry();
	}
	else
	{
		LightAttack();
	}
}
void APlayerCharacter::LightAttack()
{
	if (!bCanAttack)
	{
		return;
	}

	if (!bIsAttacking && CanMove()) // 첫 공격 시작
	{
		bIsAttacking = true;
		CurrentAttackCombo = 1;
		PlayLightAttackMontage();
		return;
	}

	if (bCanCombo) // 플레이어가 콤보 연속 입력 시
	{
		bWantCombo = true;
	}
}

void APlayerCharacter::PlayLightAttackMontage()
{
	ResetHitCharacters(); // 피격 캐릭터들, 리액션 가능하도록 초기화
	bCanCombo = false;
	AddStamina(-StaminaLightAttackCost);
	UAnimMontage *CurrentAnimMontage;
	if (GetStamina() < StaminaLightAttackCost)
	{
		CurrentAttackCombo = 0;
		bIsAttacking = false;
		ChangeMovement(EMovementState::MS_Idle);
		return;
	}

	if (bIsRunning && GetVelocity().Size() >= (RunSpeed * 0.85f)) // 플레이어 캐릭터 현재 속도가 달리기 속도의 85% 이상일 때, 대쉬 공격 재생
	{
		CurrentAnimMontage = RunAttackMontage;
		CurrentAttackCombo = 1;
	}
	else
	{
		CurrentAnimMontage = AttackMontages[CurrentAttackCombo - 1];
	}

	if (CurrentAnimMontage && !bIsDodging)
	{
		bIsAttacking = true;
		ChangeMovement(EMovementState::MS_Attacking);

		PlayAnimMontage(CurrentAnimMontage, 0.9f);
	}
}
void APlayerCharacter::Parry()
{
	if (ParryMontage)
	{
		bIsParrying = true;
		bIsGuarding = false;
		PlayAnimMontage(ParryMontage, 1.0f);
	}
}
void APlayerCharacter::DoBlockHitReaction()
{
	CurrentAttackCombo = 0;
	PlayAnimMontage(BlockAttackMontage, 1.0f);
	AddStamina(-10.0f);
	bCanAttack = false;
	bIsAttacking = false;
	bCanCombo = false;
}
void APlayerCharacter::Stun()
{
	bIsAttacking = false;
	bCanAttack = false;
	if (StunMontage)
	{
		ChangeMovement(EMovementState::MS_Stun);
		PlayAnimMontage(StunMontage, 1.0f);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("No StunMontage!"));
	}
}

void APlayerCharacter::DoHitReaction(EAttackDirection ad)
{
	int32 index = 0;
	bIsAttacking = false; // 피격 중 공격 판정 방지
	switch (ad)
	{
	case EAttackDirection::AD_Left:
		index = 0;
		break;
	case EAttackDirection::AD_Right:
		index = 1;
		break;
	case EAttackDirection::AD_Forward:
		index = 2;
		break;
	}
	PlayHitMontage(index);
}

void APlayerCharacter::PlayHitMontage(int32 index)
{
	UAnimMontage *CurrentAnimMontage = (bIsGuarding ? HitGuardMontages[index] : HitMontages[index]);
	bIsHit = true;
	if (CurrentAnimMontage)
	{
		PlayAnimMontage(CurrentAnimMontage, 1.0f);
		// UE_LOG(LogTemp, Log, TEXT("Hit Montage[%d] Played"), index);
	}
}

void APlayerCharacter::Landed(const FHitResult &Hit)
{
	Super::Landed(Hit);
	bJustLanded = true;
	ChangeMovement(EMovementState::MS_Idle);
}

void APlayerCharacter::Jump()
{
	if (!CanMove()) // 공격 중 Jump 방지
	{
		return;
	}
	Super::Jump();
}

void APlayerCharacter::Run()
{
	if (!bIsRunning && GetStamina() >= StaminaRunStartCost && CanMove())
	{
		AddStamina(-StaminaRunStartCost);
		bIsGuarding = false;
		bIsRunning = true;
		MoveComp->MaxWalkSpeed = RunSpeed;
	}
}

void APlayerCharacter::StopRunning()
{
	if (bIsRunning)
	{
		bIsRunning = false;
		bIsDodging = false;
		MoveComp->MaxWalkSpeed = WalkSpeed;
	}
}

void APlayerCharacter::Dodge()
{
	if (!bIsFocusing || bIsDodging || !CanMove() || GetStamina() < StaminaDodgeCost)
		return;
	bWantCombo = false;
	AddStamina(-StaminaDodgeCost);
	ChangeMovement(EMovementState::MS_Dodging);
	bIsDodging = true;
	PlayAnimMontage(DodgeMontage, 1.2f);
}

bool APlayerCharacter::CanMove()
{
	return (MovementState == EMovementState::MS_Moving ||
			MovementState == EMovementState::MS_Idle ||
			MovementState == EMovementState::MS_Guarding);
}

void APlayerCharacter::ResetMovement()
{
	ChangeMovement(EMovementState::MS_Idle);
	bIsParrying = false;
	bIsRunning = false;
	bIsDodging = false;
	bIsAttacking = false;
	bCanAttack = true;
	CurrentAttackCombo = 0;
	// bIsHit = false;
}
void APlayerCharacter::ChangeMovement(EMovementState ms)
{
	if (MovementState != ms)
	{
		MovementState = ms;
		if (GEngine)
		{
			// 고정 키: 중복 출력 방지 및 삭제용
			static const int32 EnumDebugKey = 1001;

			// 이전 메시지 삭제 (있으면 삭제, 없어도 안전)
			GEngine->RemoveOnScreenDebugMessage(EnumDebugKey);

			// 출력
			FText Display = EnumDisplayName(MovementState);
			GEngine->AddOnScreenDebugMessage(EnumDebugKey, 10.0f, FColor::Yellow, Display.ToString());
		}
	}
}
void APlayerCharacter::Guard()
{
	if (!bIsGuarding && GetStamina() > 5)
	{
		bIsGuarding = true;
		MoveComp->MaxWalkSpeed = WalkSpeed * 0.8;
		ChangeMovement(EMovementState::MS_Guarding);
	}
}

void APlayerCharacter::StopGuarding()
{
	if (bIsGuarding)
	{
		UE_LOG(LogTemp, Log, TEXT("StopGuarding"));
		bIsGuarding = false;
		MoveComp->MaxWalkSpeed = WalkSpeed;
		ChangeMovement(EMovementState::MS_Idle);
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
			FVector Forward = SwordMeshComponent->GetUpVector();
			FVector End = Start + Forward * LightAttackRange;

			// 쿼리 파라미터 (자기 자신 무시)
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(this);
			QueryParams.bTraceComplex = true;

			// Pawn 오브젝트 타입만 검사하도록 설정
			FCollisionObjectQueryParams ObjectQueryParams;
			ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
			TArray<FHitResult> Hits;
			float SphereRadius = 10.0f;

			bool bHit = World->SweepMultiByObjectType(
				Hits,
				Start,
				End,
				FQuat::Identity,
				ObjectQueryParams,
				FCollisionShape::MakeSphere(SphereRadius),
				QueryParams);
			if (bHit)
			{
				for (auto Hit : Hits)
				{
					APlayerCharacter *HitPlayerCharacter = nullptr;
					if (bHit)
					{
						HitPlayerCharacter = Cast<APlayerCharacter>(Hit.GetActor());
						if (HitPlayerCharacter)
						{
							if (HitPlayerCharacter->bIsHit)
							{
								continue;
							}
							HitPlayerCharacter->bIsHit = true;
							HitPlayerCharacter->ChangeMovement(EMovementState::MS_Hit);
							HitCharacters.Add(HitPlayerCharacter);
							UE_LOG(LogTemp, Log, TEXT("[AttackTrace] Hit Pawn Actor: %s"),
								   *Hit.GetActor()->GetName());
							EAttackDirection AttackDirection = EAttackDirection::AD_Forward;
							switch (CurrentAttackCombo) // 콤보에 따라 공격 방향 설정
							{
							case 1:
							case 3:
								AttackDirection = EAttackDirection::AD_Left;
								break;
							case 2:
								AttackDirection = EAttackDirection::AD_Right;
								break;
							case 4:
								AttackDirection = EAttackDirection::AD_Forward;
								break;
							default:
								break;
							}
							if (!HitPlayerCharacter->bIsParrying)
							{
								HitPlayerCharacter->DoHitReaction(AttackDirection);
							}

							if (HitPlayerCharacter->bIsParrying)
							{
								Stun();
							}
							else if (HitPlayerCharacter->bIsGuarding)
							{
								HitPlayerCharacter->AddHealth(-LightAttackDamage * 0.3);
								HitPlayerCharacter->AddStamina(-10.0f);
							}
							else
							{
								HitPlayerCharacter->AddHealth(-LightAttackDamage);
							}
							/*
							// 공격자(this) 위치에서 피격자 위치로의 벡터 (피격자가 공격자를 바라보려면 공격자 - 피격자)
							FVector ToAttacker = GetActorLocation() - HitPlayerCharacter->GetActorLocation();
							ToAttacker.Z = 0.f;

							if (!ToAttacker.IsNearlyZero()) // 맞은 캐릭터 시점 고정하기
							{
								FRotator LookAtRot = ToAttacker.Rotation();
								LookAtRot.Pitch = 0.f;
								LookAtRot.Roll = 0.f;

								if (AController *HitCtrl = HitPlayerCharacter->GetController())
								{
									HitCtrl->SetControlRotation(LookAtRot);
								}

								// 액터 자체 회전도 설정
								HitPlayerCharacter->SetActorRotation(LookAtRot);
								// HitPlayerCharacter->GetCharacterMovement()->AddImpulse(FVector(100.0f, 0, 0), false);
							}
								*/
							if (HitPlayerCharacter->bIsGuarding && BlockAttackMontage && !HitPlayerCharacter->bIsParrying) // 방어 중인 공격자를 만났을 때
							{
								DoBlockHitReaction();
							}
						}
					}
				}
			}
			// DrawDebugLine(World, Start, End, bHit ? FColor::Red : FColor::Green, false, 1.0f, 0, 1.0f);
			// DrawDebugSphere(World, Start, SphereRadius, 12, FColor::Blue, false, 1.0f);
			// DrawDebugSphere(World, End, SphereRadius, 12, FColor::Blue, false, 1.0f);
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

void APlayerCharacter::ResetHitCharacters()
{
	for (auto ch : HitCharacters)
	{
		ch->bIsHit = false;
	}
	HitCharacters.Empty();
}

void APlayerCharacter::AddHealth(float Amount)
{
	if (Health + Amount < 0)
	{
		Die();
		Health = 0;
	}
	else
	{
		if (Health > MaxHealth)
		{
			Health = MaxHealth;
		}
	}
	Health += Amount;
	StatusBar->SetHealthPercent(Health / MaxHealth);
}
float APlayerCharacter::GetHealth()
{
	return Health;
}
void APlayerCharacter::AddStamina(float Amount)
{
	if (Stamina + Amount < 0)
	{
		Stamina = 0;
		StopGuarding();
		StopRunning();
	}
	else
	{
		Stamina += Amount;
		if (Stamina > MaxStamina)
		{
			Stamina = MaxStamina;
		}
	}
	StatusBar->SetStaminaPercent(Stamina / MaxStamina);
}
float APlayerCharacter::GetStamina()
{
	return Stamina;
}
void APlayerCharacter::Die()
{
	UE_LOG(LogTemp, Warning, TEXT("%s is Dead."), *GetName());
}
