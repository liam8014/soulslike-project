// Fill out your copyright notice in the Description page of Project Settings.

#include "AOEExplosion.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "Particles/ParticleSystem.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "SoulsLike/SoulsLikesTypes.h"
#include "SoulsLike/ObjectPooling/PoolManagerSubsystem.h"
#include "SoulsLike/Character/PlayerCharacter.h"

AAOEExplosion::AAOEExplosion()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

void AAOEExplosion::OnSpawnFromPool()
{
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	// SetActorTickEnabled(true);
	UE_LOG(LogTemp, Warning, TEXT("AOE Actor Spawned!(POOL)"));
	TRACE_BOOKMARK(TEXT("AOE_Spawn_Pool"));
	PoolMgr = GetWorld()->GetSubsystem<UPoolManagerSubsystem>();
	if (BeforeEffect1Class)
	{
		if (PoolMgr)
		{
			PoolMgr->SpawnFromPool(BeforeEffect1Class, GetActorLocation(), GetActorRotation());
		}
	}

	if (BeforeEffect2Class)
	{
		if (PoolMgr)
		{
			PoolMgr->SpawnFromPool(BeforeEffect2Class, GetActorLocation(), GetActorRotation());
		}
	}

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &AAOEExplosion::Explode, ExplosionDelay, false);
}

void AAOEExplosion::OnReturnToPool()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);

	GetWorldTimerManager().ClearAllTimersForObject(this); // 타이머 해제
}

// Called when the game starts or when spawned
void AAOEExplosion::BeginPlay()
{
	Super::BeginPlay();
	if (!bUsePooling)
	{
		TRACE_BOOKMARK(TEXT("AOE_Spawn"));
		if (BeforeEffect1)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeforeEffect1, GetActorLocation(), GetActorRotation(), FVector(1.0f), true);
		}
		if (BeforeEffect2)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeforeEffect2, GetActorLocation(), GetActorRotation(), FVector(1.0f), true);
		}
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &AAOEExplosion::Explode, ExplosionDelay, false);
	}
}

// Called every frame
void AAOEExplosion::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAOEExplosion::Explode()
{
	if (ExplosionEffect && HasAuthority())
	{
		if (bUsePooling && ExplosionEffectClass && PoolMgr)
		{
			PoolMgr->SpawnFromPool(ExplosionEffectClass, GetActorLocation(), GetActorRotation());
		}
		else
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation(), GetActorRotation(), FVector(1.0f));
		}

		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &AAOEExplosion::AttackTrace, TraceDelay, false);
	}
}

void AAOEExplosion::AttackTrace()
{
	if (ExplosionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, GetActorLocation());
	}
	// UE_LOG(LogTemp, Log, TEXT("Trace"));
	TSet<AActor *> ProcessedActors;
	TArray<FOverlapResult> OverlapResults;
	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(AttackRadius);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetInstigator());

	bool bHasOverlap = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		GetActorLocation(),
		FQuat::Identity,
		ECC_Pawn,
		CollisionShape,
		QueryParams);

	if (bHasOverlap)
	{
		for (const FOverlapResult &HitResult : OverlapResults)
		{
			AActor *OverlappedActor = HitResult.GetActor();
			if (ProcessedActors.Contains(OverlappedActor))
				continue;
			else
				ProcessedActors.Add(OverlappedActor);
			if (OverlappedActor && OverlappedActor != this)
			{
				APlayerCharacter *PlayerChar = Cast<APlayerCharacter>(OverlappedActor);

				if (PlayerChar)
				{
					FVector ToSource = (GetActorLocation() - PlayerChar->GetActorLocation());
					ToSource.Z = 0.0f;
					ToSource.Normalize();

					FVector PlayerForward = PlayerChar->GetActorForwardVector();
					PlayerForward.Z = 0.0f;
					PlayerForward.Normalize();

					FVector PlayerRight = PlayerChar->GetActorRightVector();
					PlayerRight.Z = 0.0f;
					PlayerRight.Normalize();

					float ForwardDot = FVector::DotProduct(PlayerForward, ToSource);
					float RightDot = FVector::DotProduct(PlayerRight, ToSource);

					EAttackDirection HitDir = EAttackDirection::AD_Forward;

					if (FMath::Abs(ForwardDot) > FMath::Abs(RightDot))
					{
						// 앞/뒤가 더 가까움
						if (ForwardDot > 0.0f)
						{
							HitDir = EAttackDirection::AD_Forward; // 플레이어 기준 정면
						}
						else
						{
							HitDir = EAttackDirection::AD_Backward; // 플레이어 기준 후면
						}
					}
					else
					{
						// 좌/우가 더 가까움
						if (RightDot > 0.0f)
						{
							HitDir = EAttackDirection::AD_Right; // 플레이어 기준 우측
						}
						else
						{
							HitDir = EAttackDirection::AD_Left; // 플레이어 기준 좌측
						}
					}

					FVector ExplosionCenter = this->GetActorLocation();
					FVector TargetLocation = FVector::ZeroVector;

					UCapsuleComponent *TargetCapsule = PlayerChar->GetCapsuleComponent();
					if (TargetCapsule)
					{
						TargetCapsule->GetClosestPointOnCollision(ExplosionCenter, TargetLocation);
					}
					else
					{
						TargetLocation = PlayerChar->GetActorLocation();
					}

					FGameplayHitInfo HitInfo;
					HitInfo.DamageAmount = DamageAmount;
					HitInfo.KnockBackDistance = 0;
					HitInfo.AttackDirection = HitDir;
					HitInfo.HitLocation = TargetLocation;
					HitInfo.ImpactNormal = (ExplosionCenter - TargetLocation).GetSafeNormal();
					HitInfo.DamageCauser = nullptr;
					HitInfo.bCanBlock = false;
					PlayerChar->Hit(HitInfo);
				}
			}
		}
	}
	// DrawDebugSphere(GetWorld(), GetActorLocation(), AttackRadius, 32, FColor::Red, false, 2.0f);
	if (bUsePooling)
	{
		if (UWorld *World = GetWorld())
		{
			if (PoolMgr)
			{
				PoolMgr->ReturnToPool(this);
				TRACE_BOOKMARK(TEXT("AOE_Return_Pooling"));
				return; // 반환했으므로 함수 종료
			}
		}
	}
	TRACE_BOOKMARK(TEXT("AOE_Destroy"));
	Destroy(); // 혹시 모르니까 유지
}