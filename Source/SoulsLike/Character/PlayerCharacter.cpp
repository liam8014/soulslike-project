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

#include "Engine/World.h"	   // UWorld
#include "Containers/Ticker.h" // FTSTicker, FTickerDelegate
#include "SoulsLike/UI/StatusBar.h"
#include "SoulsLike/Enemy/EnemyBase.h"
#include "SoulsLike/Component/AttributeComponent.h"
#include "SoulsLike/Character/PlayerAttributeComponent.h"
#include "SoulsLike/Character/PlayerCombatComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void APlayerCharacter::RestoreFOV()
{
	TargetFOV = DefaultFOV;
}

void APlayerCharacter::ChangeFOV(float FOV)
{
	if (FollowCamera)
	{
		TargetFOV = FOV;

		GetWorldTimerManager().SetTimer(
			FOVRestoreTimerHandle,
			this,
			&APlayerCharacter::RestoreFOV,
			0.25f,
			false);
	}
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	InitComponents();
	SetupBindings();
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

	MoveComp = GetCharacterMovement();
	if (MoveComp)
	{
		MoveComp->MaxWalkSpeed = AttrComp->GetMaxWalkSpeed();
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

	if (StatusBarClass)
	{
		StatusBar = CreateWidget<UStatusBar>(GetWorld(), StatusBarClass);
		if (StatusBar)
		{
			StatusBar->AddToViewport();
		}
	}

	if (CameraBoom)
	{
		DefaultSocketOffset = CameraBoom->SocketOffset;
	}
	if (FollowCamera)
	{
		DefaultFOV = FollowCamera->FieldOfView;
		TargetFOV = DefaultFOV; // 초기 목표는 기본값
	}
}
void APlayerCharacter::InitComponents()
{
	AttrComp = FindComponentByClass<UPlayerAttributeComponent>();
	if (AttrComp)
	{
		UE_LOG(LogTemp, Display, TEXT("Attribute Component is successfully set"));
		AttrComp->ChangeHealth(0.0f);
		AttrComp->ChangeStamina(0.0f);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Attribute Component set failed!"));
	}

	CombatComp = FindComponentByClass<UPlayerCombatComponent>();
	if (CombatComp)
	{
		UE_LOG(LogTemp, Display, TEXT("Combat Component is successfully set"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Combat Component set failed!"));
	}
}
void APlayerCharacter::SetupBindings()
{
	if (UAnimInstance *AnimInst = GetMesh()->GetAnimInstance())
	{
		AnimInst->OnPlayMontageNotifyBegin.AddDynamic(this, &APlayerCharacter::OnNotifyBegin); // NotifyState 시작(NotifyBegin) 바인딩
		AnimInst->OnPlayMontageNotifyEnd.AddDynamic(this, &APlayerCharacter::OnNotifyEnd);	   // NotifyState 종료(NotifyEnd) 바인딩
	}
	if (AttrComp)
	{
		AttrComp->OnHealthChanged.AddDynamic(this, &APlayerCharacter::OnHealthChangedReceived);
		AttrComp->OnStaminaChanged.AddDynamic(this, &APlayerCharacter::OnStaminaChangedReceived);
	}
}
// Move 입력 시 호출되는 함수
void APlayerCharacter::Move(const FInputActionValue &Value)
{
	if (!CanMove())
	{
		return;
	}
	CombatComp->DisableCounterAttack();
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
			SetMovementState(EMovementState::MS_Moving);
			return;
		} // Facing 이상시 기본 처리

		float Dot = FVector::DotProduct(MoveDir, Facing); // 정렬도 (-1..1)
		if (Dot >= AlignmentThreshold)
		{
			AddMovementInput(MoveDir, InLen);
			SetMovementState(EMovementState::MS_Moving);
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
			SetMovementState(EMovementState::MS_Moving);
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
	if (!IsValid(Target) || FVector::Dist(TargetLoc, PlayerLoc) > FocusSearchRadius)
	{
		ToogleFocus();
		return;
	}

	FVector2D ScreenPos;
	if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PlayerController, TargetLoc, ScreenPos, false))
	{
		FocusIndicatorWidget->SetPositionInViewport(ScreenPos + FVector2D(0.0f, -30.0f), false);
	}

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
		// SetMovementState(EMovementState::MS_Falling);
	}
	else if (MovementState == EMovementState::MS_Falling)
	{
		SetMovementState(EMovementState::MS_Idle);
	}

	if (MovementState == EMovementState::MS_Moving && GetVelocity().Size() == 0)
	{
		SetMovementState(EMovementState::MS_Idle);
	}

	if (bIsRunning && CanMove())
	{
		AttrComp->ChangeStamina(-AttrComp->GetSprintStaminaCost());
	}

	if (CameraBoom)
	{
		FVector TargetOffset = CombatComp->CanCounterAttack() ? CounterReadySocketOffset : DefaultSocketOffset;

		CameraBoom->SocketOffset = FMath::VInterpTo(
			CameraBoom->SocketOffset,
			TargetOffset,
			DeltaTime,
			CameraInterpSpeed);
	}

	if (FollowCamera)
	{
		float CurrentFOV = FollowCamera->FieldOfView;
		if (!FMath::IsNearlyEqual(CurrentFOV, TargetFOV, 0.1f))
		{
			float NewFOV = FMath::FInterpTo(
				CurrentFOV,
				TargetFOV,
				DeltaTime,
				FOVInterpSpeed);
			FollowCamera->SetFieldOfView(NewFOV);
		}
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
		EnhancedInput->BindAction(HeavyAttackAction, ETriggerEvent::Started, this, &APlayerCharacter::HeavyAttack);

		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		EnhancedInput->BindAction(RunAction, ETriggerEvent::Started, this, &APlayerCharacter::Run);
		EnhancedInput->BindAction(RunAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopRunning);

		EnhancedInput->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Dodge);

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
		CombatComp->EnableAttackCombo();
	}

	if (NotifyName == TEXT("ReadyCounter"))
	{
		CombatComp->EnableAttack();
	}
	if (NotifyName == TEXT("ResetMovement"))
	{
		ResetMovement();
	}
	if (NotifyName == TEXT("Parry"))
	{
		CombatComp->EnableParry();
	}
	if (NotifyName == TEXT("Sweep"))
	{
		CombatComp->EnableAttackSweep();
	}
	if (NotifyName == TEXT("ChangeFOV"))
	{
		ChangeFOV(CounterActionFOV);
	}
}

void APlayerCharacter::OnNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload &Payload)
{
	if (NotifyName == TEXT("ComboWindow"))
	{
		CombatComp->DisableAttackCombo();
	}
	if (NotifyName == TEXT("Sweep"))
	{
		CombatComp->DisableAttackSweep();
	}
	if (NotifyName == TEXT("Parry"))
	{
		CombatComp->DisableParry();
	}
	if (NotifyName == TEXT("ReadyCounter"))
	{
		// if (ReleaseCounterMontage && !bIsAttacking && MovementState != EMovementState::MS_Moving)
		// {
		// 	PlayAnimMontage(ReleaseCounterMontage, 1.0f);
		// }
	}
}
void APlayerCharacter::Attack()
{
	AttrComp->DisableRegenStamina();
	if (CombatComp)
	{
		CombatComp->RequestAttack(); // 내부에서 Parry, Counter, LightAttack 중 판단
	}
}

void APlayerCharacter::HeavyAttack()
{
	if (CombatComp->CanAttack() && CanMove())
	{
		if (MovementState == EMovementState::MS_Attacking)
		{
			return;
		}
		if (AttrComp->TryConsumeStamina(AttrComp->GetHeavyAttackStaminaCost()))
		{
			CombatComp->HeavyAttack();
			LaunchCharacter(GetActorForwardVector() * 1500 + FVector(0, 0, 100), true, true);
		}
		else
		{
			ResetMovement();
		}
	}
}

EHitResult APlayerCharacter::Hit(const FGameplayHitInfo &HitInfo)
{
	SetMovementState(EMovementState::MS_Hit);
	return CombatComp->Hit(HitInfo);
}

void APlayerCharacter::Landed(const FHitResult &Hit)
{
	Super::Landed(Hit);
	bJustLanded = true;
	// SetMovementState(EMovementState::MS_Idle);
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
	if (!bIsRunning && CanMove())
	{
		if (AttrComp->TryConsumeStamina(AttrComp->GetSprintActivationCost()))
		{
			AttrComp->DisableRegenStamina();
			CombatComp->DisableGuard();
			bIsRunning = true;
			MoveComp->MaxWalkSpeed = AttrComp->GetMaxSprintSpeed();
		}
	}
}

void APlayerCharacter::StopRunning()
{
	if (bIsRunning)
	{
		if (!CheckMovementState(EMovementState::MS_Attacking))
		{
			AttrComp->EnableRegenStamina();
		}
		bIsRunning = false;
		MoveComp->MaxWalkSpeed = AttrComp->GetMaxWalkSpeed();
	}
}

void APlayerCharacter::Dodge()
{
	if (!bIsFocusing || !CanMove())
	{
		return;
	}
	CombatComp->Dodge();
}

bool APlayerCharacter::CanMove()
{
	return (MovementState == EMovementState::MS_Moving ||
			MovementState == EMovementState::MS_Idle);
}

void APlayerCharacter::ResetMovement()
{
	SetMovementState(EMovementState::MS_Idle);
	CombatComp->ResetMovement();
	AttrComp->EnableRegenStamina();

	bIsRunning = false;
}
void APlayerCharacter::SetMovementState(EMovementState NewMovementState)
{
	if (MovementState != NewMovementState)
	{
		MovementState = NewMovementState;
	}
}
bool APlayerCharacter::CheckMovementState(EMovementState _MovementState)
{
	return (MovementState == _MovementState);
}
EMovementState APlayerCharacter::GetMovementState()
{
	return MovementState;
}
bool APlayerCharacter::CanBeHit()
{
	return CombatComp->CanBeHit();
}
void APlayerCharacter::EnableCounterAttack()
{
	CombatComp->EnableCounterAttack();
}

void APlayerCharacter::Guard()
{
	CombatComp->Guard();
}

void APlayerCharacter::StopGuarding()
{
	CombatComp->StopGuarding();
}

void APlayerCharacter::HideUI()
{
	if (StatusBar)
	{
		StatusBar->SetVisibility(ESlateVisibility::Hidden);
		UE_LOG(LogTemp, Warning, TEXT("UI Hidden"));
	}
}

void APlayerCharacter::ShowUI()
{
	if (StatusBar)
	{
		StatusBar->SetVisibility(ESlateVisibility::Visible);
		UE_LOG(LogTemp, Warning, TEXT("UI Visible"));
	}
}

void APlayerCharacter::OnHealthChangedReceived(float CurrentHealth, float MaxHealth)
{
	if (StatusBar)
	{
		StatusBar->SetHealthPercent(CurrentHealth / MaxHealth);
	}
	if (CurrentHealth <= 0)
	{
		Die();
	}
}

void APlayerCharacter::OnStaminaChangedReceived(float CurrentStamina, float MaxStamina)
{
	if (StatusBar)
	{
		StatusBar->SetStaminaPercent(CurrentStamina / MaxStamina);
	}
	if (CurrentStamina <= 0)
	{
		StopGuarding();
		StopRunning();
	}
}

void APlayerCharacter::Die()
{
	UE_LOG(LogTemp, Warning, TEXT("%s is Dead."), *GetName());
}
