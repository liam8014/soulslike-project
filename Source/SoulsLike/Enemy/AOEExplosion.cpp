// Fill out your copyright notice in the Description page of Project Settings.

#include "AOEExplosion.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "SoulsLike/SoulsLikesTypes.h"
#include "SoulsLike/Character/PlayerCharacter.h"

AAOEExplosion::AAOEExplosion()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AAOEExplosion::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("AOE Actor Spawned!"));
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

// Called every frame
void AAOEExplosion::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAOEExplosion::Explode()
{
	if (ExplosionEffect && HasAuthority())
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation(), GetActorRotation(), FVector(1.0f));
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &AAOEExplosion::AttackTrace, TraceDelay, false);
	}
}

void AAOEExplosion::AttackTrace()
{
	UE_LOG(LogTemp, Log, TEXT("Trace"));
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
		for (const FOverlapResult &Result : OverlapResults)
		{
			AActor *OverlappedActor = Result.GetActor();
			if (ProcessedActors.Contains(OverlappedActor))
				continue;
			else
				ProcessedActors.Add(OverlappedActor);
			if (OverlappedActor && OverlappedActor != this)
			{
				APlayerCharacter *PlayerChar = Cast<APlayerCharacter>(OverlappedActor);

				if (PlayerChar)
				{
					// 1. 플레이어 -> 폭발 위치로 향하는 방향 벡터 (Z축 무시하여 평면상 방향만 고려)
					FVector ToSource = (GetActorLocation() - PlayerChar->GetActorLocation());
					ToSource.Z = 0.0f;
					ToSource.Normalize();

					// 2. 플레이어의 기준 벡터 (Z축 무시)
					FVector PlayerForward = PlayerChar->GetActorForwardVector();
					PlayerForward.Z = 0.0f;
					PlayerForward.Normalize();

					FVector PlayerRight = PlayerChar->GetActorRightVector();
					PlayerRight.Z = 0.0f;
					PlayerRight.Normalize();

					// 3. 내적(Dot Product) 계산
					// ForwardDot > 0 이면 앞, < 0 이면 뒤
					// RightDot > 0 이면 오른쪽, < 0 이면 왼쪽
					float ForwardDot = FVector::DotProduct(PlayerForward, ToSource);
					float RightDot = FVector::DotProduct(PlayerRight, ToSource);

					EAttackDirection HitDir = EAttackDirection::AD_Forward;

					// 4. 더 크게 영향을 미치는 축(앞뒤 vs 좌우)을 찾아서 방향 결정
					// 절대값을 비교하여 어느 쪽이 더 지배적인지 확인합니다.
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

					PlayerChar->Hit(DamageAmount, 0, HitDir);
					UE_LOG(LogTemp, Log, TEXT("Player Hit! Name: %s, Direction: %d"), *PlayerChar->GetName(), (int32)HitDir);
				}
			}
		}
	}

	DrawDebugSphere(GetWorld(), GetActorLocation(), AttackRadius, 32, FColor::Red, false, 2.0f);
	Destroy();
}