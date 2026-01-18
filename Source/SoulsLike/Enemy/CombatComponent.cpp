// Fill out your copyright notice in the Description page of Project Settings.

#include "CombatComponent.h"
#include "GameFramework/Character.h"
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
	UE_LOG(LogTemp, Warning, TEXT("EnableAttackSweep"));
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
}

void UCombatComponent::AttackTrace()
{
	if (!Owner)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to find Owner"));
		DisableAttackSweep();
		return;
	}

	USkeletalMeshComponent *Mesh = Owner->GetMesh();
	if (!Mesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AttackTrace] Mesh is null"));
		bIsAttacking = false;
		return;
	}

	UWorld *World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AttackTrace] GetWorld() returned null"));
		bIsAttacking = false;
		return;
	}
	if (AttackRange <= 0.0f || AttackRadius <= 0.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("Range or Radius is ZERO! Range: %f, Radius: %f"), AttackRange, AttackRadius);
		return;
	}
	FVector Start = Mesh->GetSocketLocation(FName("weapon"));
	FVector Forward = -Mesh->GetRightVector();
	FVector End = Start + Forward * AttackRange;

	FCollisionShape Shape = FCollisionShape::MakeSphere(AttackRadius);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);
	QueryParams.bTraceComplex = true;

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	TArray<FHitResult> Hits;

	bool bHit = World->SweepMultiByObjectType(
		Hits,
		Start,
		End,
		FQuat::Identity,
		ObjectQueryParams,
		Shape,
		QueryParams);

	if (bDrawDebugShape)
	{
		DrawDebugSphere(World, Start, AttackRadius, 12, FColor::Green, false, 1.0f);
		DrawDebugSphere(World, End, AttackRadius, 12, FColor::Red, false, 1.0f);
	}

	if (!bHit)
	{
		return;
	}

	for (const FHitResult &H : Hits)
	{
		AActor *HitActor = H.GetActor();

		if (ProcessedActors.Contains(HitActor))
		{
			continue;
		}

		APlayerCharacter *HitPlayerCharacter = Cast<APlayerCharacter>(HitActor);

		if (HitPlayerCharacter && HitPlayerCharacter->bCanBeHit)
		{
			ProcessedActors.Add(HitActor);

			if (!HitPlayerCharacter->bIsFocusing)
			{
				FVector ToAttacker = Owner->GetActorLocation() - HitPlayerCharacter->GetActorLocation();
				ToAttacker.Z = 0.f;

				if (!ToAttacker.IsNearlyZero())
				{
					FRotator LookAtRot = ToAttacker.Rotation();
					LookAtRot.Pitch = 0.f;
					LookAtRot.Roll = 0.f;

					if (AController *HitCtrl = HitPlayerCharacter->GetController())
					{
						HitCtrl->SetControlRotation(LookAtRot);
					}
					HitPlayerCharacter->SetActorRotation(LookAtRot);
				}
			}

			switch (HitPlayerCharacter->Hit(DamageMultiplier * AttackPower, KnockBackDistance, AttackDirection))
			{
			case EHitResult::HR_Parry:
				break;
			case EHitResult::HR_Guard:
				break;
			}
		}
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

	const float FrontOffset = 50.0f; // 액터 앞쪽으로 박스 시작 지점까지의 거리
	const float SweepRange = 150.0f; // 박스가 스윕할 총 거리

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
	NextAttackType = FMath::RandRange(0, AttackTypeAttributes.Num() - 2);
	if (AttackTypeAttributes.IsValidIndex(NextAttackType))
	{
		NextAcceptance = AttackTypeAttributes[NextAttackType].MinimumDistance;
		NextWaitTime = AttackTypeAttributes[NextAttackType].WaitTime;
		NextAnimMontage = AttackTypeAttributes[NextAttackType].AttackAnim;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to set Next Attack Type!"));
	}
}

void UCombatComponent::SetAttackAttribute(float Multiplier, float KnockBack, EAttackDirection Direction)
{
	if (Multiplier != -1)
	{
		DamageMultiplier = Multiplier;
	}
	if (KnockBack != -1)
	{
		KnockBackDistance = KnockBack;
	}
	AttackDirection = Direction;
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
