// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter.h"

#include "EnhancedInputComponent.h"	  // UEnhancedInputComponent
#include "EnhancedInputSubsystems.h"  // UEnhancedInputLocalPlayerSubsystem
#include "InputActionValue.h"		  // FInputActionValue
#include "GameFramework/Controller.h" // APlayerController
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h" // UInputComponent base
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"

#include "GameFramework/SpringArmComponent.h" // USpringArmComponent
#include "Camera/CameraComponent.h"			  // UCameraComponent
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationYaw = false; // 컨트롤러 회전(Yaw) 사용하지 않음
	// 이동 방향으로 자동 회전
	if (UCharacterMovementComponent *MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->RotationRate = FRotator(0.f, 500.0f, 0.f);
	}
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

	CameraBoom = FindComponentByClass<USpringArmComponent>();
	FollowCamera = FindComponentByClass<UCameraComponent>();
}

// Move 입력 시 호출되는 함수
void APlayerCharacter::Move(const FInputActionValue &Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (PlayerController && (MovementVector.X != 0.f || MovementVector.Y != 0.f))
	{
		const FRotator YawRotation(0, PlayerController->GetControlRotation().Yaw, 0);
		const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDir, MovementVector.Y);
		AddMovementInput(RightDir, MovementVector.X);
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
		return false;

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
		return false;

	// 5) 히트 결과 순회하며 Pawn만 배열에 추가
	for (const FHitResult &HR : HitResults)
	{
		if (APawn *P = Cast<APawn>(HR.GetActor()))
		{
			FocusTargetArray.AddUnique(P);
			UE_LOG(LogTemp, Display, TEXT("Sweep hit pawn: %s"), *P->GetName());
		}
	}

	if (FocusTargetArray.Num() == 0)
		return false;

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

	// 1) 타겟 위치 구하기 (상체 레벨 오프셋 적용 가능)
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
		FocusIndicatorWidget->SetPositionInViewport(ScreenPos + FVector2D(0.0f, -15.0f), false);

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
		if (UCharacterMovementComponent *MoveComp = GetCharacterMovement())
		{
			MoveComp->bOrientRotationToMovement = true; // 이동 방향으로 자동 회전
		}
	}
	else if (SearchFocusTarget())
	{
		bIsFocusing = true;
		FocusIndicatorWidget->SetVisibility(ESlateVisibility::Visible);

		bUseControllerRotationYaw = true; // 컨트롤러 회전 사용
		if (UCharacterMovementComponent *MoveComp = GetCharacterMovement())
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
}

// Called to bind functionality to input
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
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to bind MoveAction!"));
	}
}
