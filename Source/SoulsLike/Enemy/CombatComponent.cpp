// Fill out your copyright notice in the Description page of Project Settings.

#include "CombatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "SoulsLike/Component/AttributeComponent.h"
#include "EnemyBase.h"
// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

void UCombatComponent::EnableAttackSweep()
{
	bIsSweeping = true;
	ProcessedActors.Empty();
}

void UCombatComponent::DisableAttackSweep()
{
	bIsSweeping = false;
}

// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	Owner = Cast<AEnemyBase>(GetOwner());
	if (Owner == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to set Owner"));
	}
	SetRandomAttackType();
	OwnerAttribute = GetOwner()->FindComponentByClass<UAttributeComponent>();
}

void UCombatComponent::AttackTrace()
{
	if (!Owner || !Owner->GetMesh() || !GetWorld())
		return;

	TArray<FHitResult> Hits;
	bool bHit = PerformAttackSweep(Hits);

	if (!bHit)
		return;

	for (const FHitResult &H : Hits)
	{
		ProcessHit(H);
	}
}

bool UCombatComponent::PerformAttackSweep(TArray<FHitResult> &OutHits)
{
	if (AttackRange <= 0.0f || AttackRadius <= 0.0f)
		return false;

	USkeletalMeshComponent *Mesh = Owner->GetMesh();
	FVector Start = Mesh->GetSocketLocation(TraceSocket);
	FVector Forward = -Mesh->GetSocketRotation(TraceSocket).Vector();
	FVector End = Start + Forward * AttackRange;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);
	QueryParams.bTraceComplex = true;

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	bool bHit = GetWorld()->SweepMultiByObjectType(
		OutHits,
		Start,
		End,
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(AttackRadius),
		QueryParams);

	if (bDrawDebugShape)
	{
		DrawDebugSphere(GetWorld(), Start, AttackRadius, 12, FColor::Green, false, 1.0f);
		DrawDebugSphere(GetWorld(), End, AttackRadius, 12, FColor::Red, false, 1.0f);
	}

	return bHit;
}

void UCombatComponent::ProcessHit(const FHitResult &HitResult)
{
	AActor *HitActor = HitResult.GetActor();

	if (!HitActor || ProcessedActors.Contains(HitActor))
		return;

	APlayerCharacter *Player = Cast<APlayerCharacter>(HitActor);
	if (!Player || !Player->bCanBeHit)
		return;

	ProcessedActors.Add(HitActor);

	if (!Player->bIsFocusing)
	{
		FVector ToAttacker = Owner->GetActorLocation() - Player->GetActorLocation();
		ToAttacker.Z = 0.f;
		if (!ToAttacker.IsNearlyZero())
		{
			FRotator LookAtRot = ToAttacker.Rotation();
			LookAtRot.Pitch = 0.f;
			LookAtRot.Roll = 0.f;

			Player->SetActorRotation(LookAtRot);
			if (AController *PC = Player->GetController())
			{
				PC->SetControlRotation(LookAtRot);
			}
		}
	}

	FGameplayHitInfo HitInfo;
	HitInfo.DamageAmount = DamageMultiplier * OwnerAttribute->GetBaseAttackPower();
	HitInfo.KnockBackDistance = KnockBackDistance;
	HitInfo.AttackDirection = AttackDirection;
	HitInfo.HitLocation = HitResult.ImpactPoint;
	HitInfo.ImpactNormal = HitResult.ImpactNormal;
	HitInfo.DamageCauser = Owner;
	HitInfo.bCanBlock = true;

	EHitResult HitType = Player->Hit(HitInfo);

	if (HitType == EHitResult::HR_Parry && Owner->AttributeComp)
	{
		Owner->AttributeComp->ChangeStamina(-30.0f);
	}

	SpawnImpactVFX(HitResult, HitType);
}

void UCombatComponent::SpawnImpactVFX(const FHitResult &Hit, EHitResult HitType)
{
	if (HitType == EHitResult::HR_CleanHit)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			AttackImpactVFX,
			Hit.ImpactPoint,
			Hit.ImpactNormal.Rotation(),
			true);
	}
}

void UCombatComponent::AttackBeforeTrace()
{

	UWorld *World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AttackBeforeTrace] GetWorld() returned null"));
		return;
	}

	FVector Start = Owner->GetActorLocation();
	FVector Forward = Owner->GetActorForwardVector();

	const float FrontOffset = 50.0f;
	const float SweepRange = 150.0f;

	const FVector HalfExtents = FVector(30.0f, 60.0f, 80.0f);

	FVector BoxStart = Start + Forward * FrontOffset;
	FVector BoxEnd = BoxStart + Forward * SweepRange;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);
	QueryParams.bTraceComplex = true;

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	FQuat BoxQuat = Owner->GetActorQuat();

	FHitResult Hit;
	bool bHit = World->SweepSingleByObjectType(
		Hit,
		BoxStart,
		BoxEnd,
		BoxQuat,
		ObjectQueryParams,
		FCollisionShape::MakeBox(HalfExtents),
		QueryParams);

	if (bHit && Hit.GetActor())
	{
		APlayerCharacter *HitPlayerCharacter = Cast<APlayerCharacter>(Hit.GetActor());
		if (HitPlayerCharacter)
		{
			HitPlayerCharacter->bIsCounterTiming = true;
		}
	}
	if (bDrawDebugShape)
	{
		const float DebugDuration = 0.5f;

		FVector SweepCenter = BoxStart + (BoxEnd - BoxStart) * 0.5f;
		FVector SweepHalfExtents = FVector(HalfExtents.X + (SweepRange * 0.5f), HalfExtents.Y, HalfExtents.Z);
		FQuat DebugQuat = BoxQuat;

		DrawDebugBox(World, BoxStart, HalfExtents, BoxQuat, FColor::Green, false, DebugDuration);

		DrawDebugBox(World, BoxEnd, HalfExtents, BoxQuat, FColor::Red, false, DebugDuration);
		DrawDebugLine(World, BoxStart, BoxEnd, FColor::Yellow, false, DebugDuration);
		if (bHit)
		{
			DrawDebugPoint(World, Hit.Location, 10.0f, FColor::Orange, false, DebugDuration);
		}
	}
}

void UCombatComponent::SetRandomAttackType()
{
	NextAttackType = FMath::RandRange(0, AttackPatterns.Num() - 1);
	if (bShouldFixAttackType)
	{
		NextAttackType = FixedAttackType;
	}
	if (AttackPatterns.IsValidIndex(NextAttackType))
	{
		NextAcceptance = AttackPatterns[NextAttackType].MinimumDistance;
		NextWaitTime = AttackPatterns[NextAttackType].WaitTime;
		NextAnimMontage = AttackPatterns[NextAttackType].AttackAnim;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to set Next Attack Type!"));
	}
}

void UCombatComponent::SetAttackAttribute(float Multiplier, float KnockBack, EAttackDirection Direction, FName Socket)
{
	DamageMultiplier = Multiplier;
	KnockBackDistance = KnockBack;
	AttackDirection = Direction;
	TraceSocket = Socket;
}

// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bIsSweeping)
	{
		AttackTrace();
	}
}
