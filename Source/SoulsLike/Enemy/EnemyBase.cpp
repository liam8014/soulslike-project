// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyBase.h"
#include "AIController.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "CombatComponent.h"
void AEnemyBase::PlayMeshJitter(float Intensity, float Duration)
{
	if (!GetMesh())
		return;

	CurrentJitterIntensity = Intensity;

	GetWorld()->GetTimerManager().ClearTimer(JitterTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(JitterRestoreTimerHandle);

	GetWorld()->GetTimerManager().SetTimer(
		JitterTimerHandle,
		this,
		&AEnemyBase::HandleMeshJitter,
		0.015f,
		true);

	GetWorld()->GetTimerManager().SetTimer(
		JitterRestoreTimerHandle,
		this,
		&AEnemyBase::RestoreMeshPosition,
		Duration,
		false);
}
void AEnemyBase::HandleMeshJitter()
{
	if (!GetMesh())
		return;

	FVector RandomOffset = FMath::VRand() * CurrentJitterIntensity;

	GetMesh()->SetRelativeLocation(OriginalMeshLocation + RandomOffset);
}
void AEnemyBase::RestoreMeshPosition()
{
	GetWorld()->GetTimerManager().ClearTimer(JitterTimerHandle);

	if (GetMesh())
	{
		GetMesh()->SetRelativeLocation(OriginalMeshLocation);
	}
}
// Sets default values
AEnemyBase::AEnemyBase()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	InitComponents();
	SetupBindings();
	InitStats();
}

void AEnemyBase::InitComponents()
{
	if (GetMesh())
	{
		OriginalMeshLocation = GetMesh()->GetRelativeLocation();
	}
	CombatComp = FindComponentByClass<UCombatComponent>();
	if (CombatComp)
	{
		UE_LOG(LogTemp, Display, TEXT("Combat Component is successfully set"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Combat Component set failed!"));
	}

	AttributeComp = FindComponentByClass<UAttributeComponent>();
	if (AttributeComp)
	{
		UE_LOG(LogTemp, Display, TEXT("Attribute Component is successfully set"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Attribute Component set failed!"));
	}
}

void AEnemyBase::SetupBindings()
{
	if (UAnimInstance *AnimInst = GetMesh()->GetAnimInstance())
	{
		AnimInst->OnPlayMontageNotifyBegin.AddDynamic(this, &AEnemyBase::OnNotifyBegin); // NotifyState 시작(NotifyBegin) 바인딩
		AnimInst->OnPlayMontageNotifyEnd.AddDynamic(this, &AEnemyBase::OnNotifyEnd);	 // NotifyState 종료(NotifyEnd) 바인딩
		UE_LOG(LogTemp, Display, TEXT("AnimInst is Set"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No AnimInst!"));
	}
}

void AEnemyBase::InitStats()
{
	if (EnemyData)
	{
		UE_LOG(LogTemp, Log, TEXT("Loading Stats from EnemyDataAsset: %s"), *EnemyData->GetName());

		// 1. AttributeComponent 스탯 적용
		if (AttributeComp)
		{
			AttributeComp->InitAttribute(EnemyData->MaxHealth, EnemyData->MaxStamina, EnemyData->StaminaRegenAmount);
		}

		// 2. CombatComponent 스탯 적용
		if (CombatComp)
		{
			CombatComp->AttackPatterns = EnemyData->AttackPatterns;
			CombatComp->SetRandomAttackType();
		}

		// 3. EnemyBase 자체 변수 적용
		this->HitSFX = EnemyData->HitSFX;
		this->DieMontage = EnemyData->DieMontage;
		this->StunMontage = EnemyData->StunMontage;
		this->DissolveSpeed = EnemyData->DissolveSpeed;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyData is NOT set! Using Default Values."));
	}
}

void AEnemyBase::OnNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload &Payload)
{
	if (NotifyName == TEXT("Sweep"))
	{
		CombatComp->AttackBeforeTrace();
		CombatComp->EnableAttackSweep();
		UE_LOG(LogTemp, Display, TEXT("Start Sweep"));
	}
	if (NotifyName == TEXT("Dash") || NotifyName == TEXT("Approach"))
	{
		AAIController *AIC = Cast<AAIController>(GetController());
		if (AIC)
		{
			FVector TargetLocation = AIC->GetFocalPoint();
			FVector MyLocation = GetActorLocation();

			FVector Direction = (TargetLocation - MyLocation);
			Direction.Z = 0.0f;
			Direction.Normalize();

			float DashSpeed = NotifyName == TEXT("Dash") ? 3500.0f : 1500.0f;
			LaunchCharacter(Direction * DashSpeed, true, false);

			FRotator LookAtRot = Direction.Rotation();
			SetActorRotation(LookAtRot);
		}
	}
	if (NotifyName == TEXT("EndStun"))
	{
		AttributeComp->EnableChangeStamina();
		if (AttributeComp->GetHealth())
		{
			bCanMove = true;
		}
		bIsStunned = false;
	}
	if (NotifyName == TEXT("EnableMove"))
	{
		bCanMove = true;
		bIsStunned = true;
	}
}

void AEnemyBase::OnNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload &Payload)
{
	if (NotifyName == TEXT("Sweep"))
	{
		CombatComp->DisableAttackSweep();
	}
}

bool AEnemyBase::PlayAttackMontage()
{
	if (bIsDead)
	{
		return false;
	}
	UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayAttackMontage] Anim Instance Error!"));
		return false;
	}
	if (CombatComp->NextAnimMontage == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayAttackMontage] Invalid Attack Type!"));
		return false;
	}
	PlayAnimMontage(CombatComp->NextAnimMontage);
	return true;
}

void AEnemyBase::StartDissolveEffect()
{
	CurrentDissolveValue = 1.1f;

	GetWorldTimerManager().SetTimer(
		DissolveTimerHandle,
		this,
		&AEnemyBase::UpdateDissolveEffect,
		0.02f,
		true);
}

void AEnemyBase::UpdateDissolveEffect()
{
	CurrentDissolveValue = FMath::FInterpConstantTo(CurrentDissolveValue, 0.0f, 0.02f, DissolveSpeed);
	for (UMaterialInstanceDynamic *Mat : MeshMIDs)
	{
		if (Mat)
		{
			Mat->SetScalarParameterValue(FName("DissolveAmount"), CurrentDissolveValue);
		}
	}

	if (FMath::IsNearlyEqual(CurrentDissolveValue, 0.0f, 0.001f))
	{
		for (UMaterialInstanceDynamic *Mat : MeshMIDs)
		{
			if (Mat)
				Mat->SetScalarParameterValue(FName("DissolveAmount"), 0.0f);
		}
		GetWorldTimerManager().ClearTimer(DissolveTimerHandle);
		Destroy();
	}
}

// Called every frame
void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AEnemyBase::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AEnemyBase::Hit(float Damage, float StaminaDamage)
{
	if (bIsDead)
	{
		return;
	}
	PlayMeshJitter(5, 0.05f);
	if (bIsStunned)
	{
		Damage *= 2;
	}
	AttributeComp->ChangeHealth(-Damage);
	AttributeComp->ChangeStamina(-StaminaDamage);

	if (HitSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSFX, GetActorLocation());
	}
}
void AEnemyBase::Die()
{
	bCanMove = false;
	bIsDead = true;
	StartDissolveEffect();
	AttributeComp->HideHealthBar();
	if (DieMontage)
	{
		PlayAnimMontage(DieMontage, 1.0f);
	}
	if (OnDie.IsBound())
	{
		OnDie.Broadcast();
	}
}
void AEnemyBase::Stun()
{
	if (StunMontage)
	{
		PlayAnimMontage(StunMontage, 1.0);
	}
	bCanMove = false;
	bIsStunned = true;
	AttributeComp->DisableChangeStaimna();
}

bool AEnemyBase::Attack()
{
	if (!CombatComp)
	{
		UE_LOG(LogTemp, Error, TEXT("CombatComp Not Found"));
		return false;
	}
	if (!PlayAttackMontage())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed To Play Attack Montage"));
		return false;
	}
	CombatComp->SetRandomAttackType();
	return true;
}
