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
	Controller = Cast<APlayerController>(GetController());
	if (Controller)
	{
		if (UEnhancedInputLocalPlayerSubsystem *Subsystem =
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Controller->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(MovingContext, 0);
			UE_LOG(LogTemp, Display, TEXT("MappingContext registered in BeginPlay"));
		}
	}

	CameraBoom = FindComponentByClass<USpringArmComponent>();
	FollowCamera = FindComponentByClass<UCameraComponent>();
}

void APlayerCharacter::Move(const FInputActionValue &Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller && (MovementVector.X != 0.f || MovementVector.Y != 0.f))
	{
		const FRotator YawRotation(0, Controller->GetControlRotation().Yaw, 0);
		const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDir, MovementVector.Y);
		AddMovementInput(RightDir, MovementVector.X);
	}
}

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

	if (!Controller)
		return false;

	// 1) 카메라 위치·방향
	FVector CamLoc;
	FRotator CamRot;
	Controller->GetPlayerViewPoint(CamLoc, CamRot); // 플레이어의 카메라 위치/회전 정보를 가져옴
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
		return;
	}

	// 1) 타겟 위치 구하기 (상체 레벨 오프셋 적용 가능)
	APawn *Target = FocusTargetArray[CurrentFocusIndex];
	FVector TargetLoc = Target->GetActorLocation();
	// 만약 상체만 바라보려면 목/척추 본 위치나 Offset을 더해 주세요:
	// TargetLoc += FVector(0,0, ChestHeightOffset);

	// 2) 카메라(또는 스프링암) 월드 위치
	FVector CamLoc = FollowCamera
						 ? FollowCamera->GetComponentLocation()
						 : CameraBoom->GetComponentLocation();

	// 3) 바라볼 회전 계산
	FRotator DesiredRot = UKismetMathLibrary::FindLookAtRotation(CamLoc, TargetLoc);

	// 4) 부드러운 보간 (원하시면 바로 Set도 가능)
	FRotator CurrentRot = Controller->GetControlRotation();
	FRotator NewRot = FMath::RInterpTo(CurrentRot, DesiredRot, DeltaTime, 10.f);

	// 5) 컨트롤러 회전에 설정 → 카메라가 따라감
	Controller->SetControlRotation(NewRot);
}

void APlayerCharacter::SwitchFocus(const FInputActionValue &Value)
{
	if (bIsFocusing)
	{
		bIsFocusing = false;
	}
	else if (SearchFocusTarget())
	{
		bIsFocusing = true;
	}

	UE_LOG(LogTemp, Display, TEXT("%s"),
		   bIsFocusing
			   ? TEXT("Now focusing is on")
			   : TEXT("Focusing is off"));
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
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to bind MoveAction!"));
	}
}
