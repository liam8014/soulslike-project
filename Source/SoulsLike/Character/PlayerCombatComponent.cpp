// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCombatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SoulsLike/Character/PlayerCharacter.h"
#include "SoulsLike/Enemy/EnemyBase.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SoulsLike/Character/PlayerAttributeComponent.h"

// Sets default values for this component's properties
UPlayerCombatComponent::UPlayerCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}
void UPlayerCombatComponent::EnableAttackSweep()
{
	bIsSweeping = true;
	ProcessedActors.Empty();
}

void UPlayerCombatComponent::DisableAttackSweep()
{
	bIsSweeping = false;
}

void UPlayerCombatComponent::EnableAttack()
{
	bCanAttack = true;
}

void UPlayerCombatComponent::DisableAttack()
{
	bCanAttack = false;
}

void UPlayerCombatComponent::EnableAttackCombo()
{
	bCanCombo = true;
}

void UPlayerCombatComponent::DisableAttackCombo()
{
	bCanCombo = false;
	if (bWantCombo && bCanAttack)
	{
		bWantCombo = false;
		LightAttack();
	}
	else
	{
		CurrentAttackCombo = 0;
		if (Owner)
		{
			Owner->ResetMovement();
		}
	}
}

void UPlayerCombatComponent::EnableParry()
{
	bIsParrying = true;
}

void UPlayerCombatComponent::DisableParry()
{
	bIsParrying = false;
}

void UPlayerCombatComponent::EnableGuard()
{
	Guard();
}

void UPlayerCombatComponent::DisableGuard()
{
	StopGuarding();
}

bool UPlayerCombatComponent::IsGuarding() const
{
	return bIsGuarding;
}

void UPlayerCombatComponent::EnableCounterAttack()
{
	bIsCounterTiming = true;
}

void UPlayerCombatComponent::DisableCounterAttack()
{
	bIsCounterTiming = false;
	bCanCounterAttack = false;
}

bool UPlayerCombatComponent::CanCounterAttack() const
{
	return bCanCounterAttack;
}

void UPlayerCombatComponent::EnableDodge()
{
	bIsDodging = true;
}

void UPlayerCombatComponent::DisableDodge()
{
	bIsDodging = false;
}

bool UPlayerCombatComponent::CanAttack() const
{
	return bCanAttack;
}

bool UPlayerCombatComponent::CanBeHit() const
{
	return bCanBeHit;
}
void UPlayerCombatComponent::RequestAttack()
{
	if (bIsGuarding)
	{
		Parry();
		return;
	}
	if (bCanCounterAttack)
	{
		CounterAttack();
		return;
	}

	// 일반 공격 로직
	if (bCanCombo && Owner->CheckMovementState(EMovementState::MS_Attacking))
	{
		bWantCombo = true;
	}
	else if (bCanAttack && !Owner->CheckMovementState(EMovementState::MS_Attacking))
	{
		LightAttack();
	}
}

void UPlayerCombatComponent::LightAttack()
{
	if (!OwnerAttrComp->TryConsumeStamina(OwnerAttrComp->GetLightAttackStaminaCost()))
	{
		Owner->ResetMovement();
		return;
	}
	Owner->SetMovementState(EMovementState::MS_Attacking);
	bCanCombo = false;

	CurrentAttackCombo = (CurrentAttackCombo + 1) % (AttackMontages.Num() + 1);
	if (!CurrentAttackCombo) // 현재 공격 콤보 0 방지
	{
		CurrentAttackCombo = 2;
	}
	switch (CurrentAttackCombo)
	{
	case 1:
	case 4:
		AttackDirection = EAttackDirection::AD_Left;
		break;
	case 2:
	case 3:
		AttackDirection = EAttackDirection::AD_Right;
		break;
	}
	SetAttackAttribute(1.0f, 1.0f, AttackDirection, LightAttackImpactVFX);
	UAnimMontage *CurrentAnimMontage = nullptr;

	if (Owner->GetVelocity().Size() >= (OwnerAttrComp->GetMaxSprintSpeed() * 0.85f)) // 플레이어 캐릭터 현재 속도가 달리기 속도의 85% 이상일 때, 대쉬 공격 재생
	{
		CurrentAnimMontage = DashAttackMontage;
		CurrentAttackCombo = 1;
	}
	else if (CurrentAttackCombo - 1 >= 0 && CurrentAttackCombo - 1 < AttackMontages.Num())
	{
		CurrentAnimMontage = AttackMontages[CurrentAttackCombo - 1];
	}

	if (CurrentAnimMontage && !bIsDodging)
	{
		Owner->PlayAnimMontage(CurrentAnimMontage, OwnerAttrComp->GetLightAttackSpeed());
	}
}

void UPlayerCombatComponent::HeavyAttack()
{
	if (HeavyAttackMontage)
	{
		OwnerAttrComp->DisableRegenStamina();
		bCanAttack = false;

		Owner->SetMovementState(EMovementState::MS_Attacking);
		SetAttackAttribute(3.0f, 3.5f, EAttackDirection::AD_Forward, HeavyAttackImpactVFX);

		Owner->PlayAnimMontage(HeavyAttackMontage, 1.0f);
	}
}

void UPlayerCombatComponent::Dodge()
{
	if (bIsDodging)
	{
		return;
	}
	if (OwnerAttrComp->TryConsumeStamina(OwnerAttrComp->GetDodgeStaminaCost()))
	{
		Owner->SetMovementState(EMovementState::MS_Dodging);
		Owner->LaunchCharacter(-Owner->GetActorForwardVector() * 3000 + FVector(0, 0, 50), true, false);

		bWantCombo = false;
		bCanBeHit = false;

		bIsDodging = true;
		Owner->PlayAnimMontage(DodgeMontage, 1.3f);
		if (bIsCounterTiming)
		{
			bCanCounterAttack = true;
		}
		else
		{
			UE_LOG(LogTemp, Display, TEXT("Dodge Just Didn't Acticated."));
		}
	}
}

void UPlayerCombatComponent::CounterAttack()
{
	if (Owner->CheckMovementState(EMovementState::MS_Dodging))
	{
		return;
	}
	bCanCounterAttack = false;
	bCanBeHit = false;
	Owner->SetMovementState(EMovementState::MS_Attacking);
	SetAttackAttribute(4.0f, 1.0f, AttackDirection, CounterAttackImpactVFX);
	if (CounterMontage)
	{
		Owner->PlayAnimMontage(CounterMontage, 1.0f);
	}

	FVector LaunchDir = Owner->GetActorForwardVector();
	LaunchDir = LaunchDir.RotateAngleAxis(-10.0f, FVector::UpVector);

	FVector LaunchVel = (LaunchDir * 2000.0f) + FVector(0, 0, 150.0f);

	Owner->LaunchCharacter(LaunchVel, true, true);
	Owner->ChangeFOV(Owner->CounterActionFOV);
}

void UPlayerCombatComponent::Parry()
{
	if (!OwnerAttrComp->TryConsumeStamina(OwnerAttrComp->GetParryStaminaCost()))
	{
		return;
	}
	StopGuarding();
	if (ParryMontage)
	{
		Owner->PlayAnimMontage(ParryMontage, 1.0f);
	}
}

void UPlayerCombatComponent::Guard()
{
	if (!bIsGuarding && OwnerAttrComp->GetStamina() >= 1 && !bIsParrying)
	{
		bIsGuarding = true;
		OwnerAttrComp->SetStaminaRegenMultiplier(OwnerAttrComp->GetGuardStaminaRegenMultiplier());
		Owner->MoveComp->MaxWalkSpeed = OwnerAttrComp->GetMaxWalkSpeed() * 0.7;
	}
}

void UPlayerCombatComponent::StopGuarding()
{
	if (bIsGuarding)
	{
		bIsGuarding = false;
		OwnerAttrComp->SetStaminaRegenMultiplier(1.0f);
		Owner->MoveComp->MaxWalkSpeed = OwnerAttrComp->GetMaxWalkSpeed();
	}
}

void UPlayerCombatComponent::Stun()
{
	if (StunMontage)
	{
		Owner->SetMovementState(EMovementState::MS_Stun);
		Owner->PlayAnimMontage(StunMontage, 1.0f);
	}
}

void UPlayerCombatComponent::ResetMovement()
{
	bIsParrying = false;
	bIsCounterTiming = false;
	if (Owner->CheckMovementState(EMovementState::MS_Moving) ||
		Owner->CheckMovementState(EMovementState::MS_Hit))
	{
		bCanCounterAttack = false;
	}
	bIsDodging = false;
	bCanAttack = true;
	bCanBeHit = true;
	CurrentAttackCombo = 0;
	DisableAttackSweep();
}

EHitResult UPlayerCombatComponent::Hit(const FGameplayHitInfo &HitInfo)
{
	EHitResult res;

	bCanAttack = false;

	bCanCounterAttack = false;
	bIsCounterTiming = false; // 닷지 저스트 실패

	FVector KnockBackVector = -Owner->GetActorForwardVector() * HitInfo.KnockBackDistance + FVector(0, 0, 100);

	if (HitInfo.bCanBlock && bIsParrying)
	{
		res = EHitResult::HR_Parry;
		Owner->LaunchCharacter(KnockBackVector * 0.5, true, true);
		OwnerAttrComp->ChangeStamina(OwnerAttrComp->GetParryStaminaReturn());
		if (ParryActivationMontage)
		{
			Owner->PlayAnimMontage(ParryActivationMontage);
		}
	}
	else
	{
		if (HitInfo.bCanBlock && bIsGuarding)
		{
			OwnerAttrComp->ChangeHealth(-HitInfo.DamageAmount * (1 - OwnerAttrComp->GetGuardRate()));
			OwnerAttrComp->ChangeStamina(-HitInfo.DamageAmount * OwnerAttrComp->GetGuardStaminaCostRate());
			res = EHitResult::HR_Guard;
		}
		else
		{
			StopGuarding();
			OwnerAttrComp->ChangeHealth(-HitInfo.DamageAmount);
			res = EHitResult::HR_CleanHit;
		}
		Owner->LaunchCharacter(KnockBackVector, true, true);
		if (OwnerAttrComp->GetStamina() > 0)
		{
			PlayHitMontage(HitInfo.AttackDirection);
		}
		else
		{
			Stun();
		}
	}
	if (res == EHitResult::HR_Guard || res == EHitResult::HR_Parry)
	{
		if (GuardImpactVFX && ParryImpactVFX)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				res == EHitResult::HR_Guard ? GuardImpactVFX : ParryImpactVFX,
				HitInfo.HitLocation,
				HitInfo.ImpactNormal.Rotation(),
				true);
		}
	}
	return res;
}

void UPlayerCombatComponent::PlayHitMontage(EAttackDirection ad)
{
	UAnimMontage *CurrentAnimMontage;
	if (HitMontages.Contains(ad) && HitGuardMontages.Contains(ad))
	{
		CurrentAnimMontage = (bIsGuarding ? HitGuardMontages[ad] : HitMontages[ad]);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid AttackDirection key: %d"), (int32)ad);
		return;
	}

	if (CurrentAnimMontage)
	{
		Owner->PlayAnimMontage(CurrentAnimMontage, 0.9f);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Hit Montage Is Not Found!"));
	}
}

// Called when the game starts
void UPlayerCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	Owner = Cast<APlayerCharacter>(GetOwner());
	if (Owner == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to set Owner"));
	}
	OwnerAttrComp = GetOwner()->FindComponentByClass<UPlayerAttributeComponent>();

	SwordMeshComponent = GetOwner()->FindComponentByClass<UStaticMeshComponent>();
	if (SwordMeshComponent)
	{
		UE_LOG(LogTemp, Display, TEXT("Sword Mesh Component Is Successfully Set : %s"), *SwordMeshComponent->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed To Set Sword Mesh Component"));
	}
	// ...
}

void UPlayerCombatComponent::AttackTrace()
{
	if (!SwordMeshComponent) // SwordMeshComponent가 valid한지 확인
	{
		UE_LOG(LogTemp, Warning, TEXT("[AttackTrace] SwordMeshComponent is null"));
		Owner->ResetMovement();
		return;
	}
	UWorld *World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AttackTrace] GetWorld() returned null"));
		Owner->ResetMovement();
		return;
	}
	TArray<FHitResult> Hits;
	bool bHit = PerformAttackSweep(Hits);
	if (!bHit)
		return;
	for (const FHitResult &H : Hits)
	{
		ProcessHit(H);
	}
}

bool UPlayerCombatComponent::PerformAttackSweep(TArray<FHitResult> &OutHits)
{
	FVector Forward = SwordMeshComponent->GetUpVector();
	FVector Start = SwordMeshComponent->GetComponentLocation();

	FVector End = Start + Forward * OwnerAttrComp->GetAttackSweepRange();

	FCollisionShape Shape = FCollisionShape::MakeSphere(OwnerAttrComp->GetAttackSweepRadius());

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);
	QueryParams.bTraceComplex = true;

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_PhysicsBody);

	bool bHit = GetWorld()->SweepMultiByObjectType(
		OutHits,
		Start,
		End,
		FQuat::Identity,
		ObjectQueryParams,
		Shape,
		QueryParams);

	if (bDrawDebugShape)
	{
		DrawDebugCapsule(GetWorld(),
						 (Start + End) * 0.5f,
						 OwnerAttrComp->GetAttackSweepRange() * 0.5f,
						 OwnerAttrComp->GetAttackSweepRadius(),
						 FRotationMatrix::MakeFromZ(Forward).ToQuat(),
						 bHit ? FColor::Red : FColor::Green, false,
						 1.0f);
	}
	return bHit;
}

void UPlayerCombatComponent::ProcessHit(const FHitResult &HitResult)
{

	AActor *HitActor = HitResult.GetActor();

	if (ProcessedActors.Contains(HitActor))
	{
		return;
	}
	AEnemyBase *HitEnemy = Cast<AEnemyBase>(HitResult.GetActor());
	if (HitEnemy)
	{
		ProcessedActors.Add(HitActor);
		const float BasePower = OwnerAttrComp->GetBaseAttackPower();
		HitEnemy->Hit(BasePower * DamageMultiplier, BasePower * StaminaMultiplier);
		FRotator ActorRotation = Owner->GetActorRotation();
		FRotator VFXRotation = FRotator::ZeroRotator;

		switch (AttackDirection)
		{
		case EAttackDirection::AD_Left:
			VFXRotation = FRotator(0.0f, ActorRotation.Yaw - 45.0f, 0.0f);
			break;
		case EAttackDirection::AD_Right:
			VFXRotation = FRotator(0.0f, ActorRotation.Yaw + 45.0f, 0.0f);
			break;
		case EAttackDirection::AD_Forward:
			VFXRotation = FRotator(0.0f, ActorRotation.Yaw + 90.0f, 70.0);
			break;
		default:
			VFXRotation = (-SwordMeshComponent->GetUpVector()).Rotation();
			break;
		}

		FVector ImpactLocation = HitResult.ImpactPoint;

		if (AttackImpactVFX)
		{
			if (ImpactLocation.IsZero())
			{
				ImpactLocation = HitEnemy->GetActorLocation();
			}
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				AttackImpactVFX,
				ImpactLocation,
				VFXRotation);
		}
		if (ImpactParticle)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				ImpactParticle,
				ImpactLocation,
				VFXRotation,
				true);
		}
	}
}

void UPlayerCombatComponent::SetAttackAttribute(float DMultiplier, float SMultiplier, EAttackDirection Direction, UNiagaraSystem *Niagara)
{
	DamageMultiplier = DMultiplier;
	StaminaMultiplier = SMultiplier;
	AttackDirection = Direction;
	AttackImpactVFX = Niagara;
}

// Called every frame
void UPlayerCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bIsSweeping)
	{
		AttackTrace();
	}

	// ...
}
