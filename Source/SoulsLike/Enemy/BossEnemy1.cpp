// Fill out your copyright notice in the Description page of Project Settings.

#include "BossEnemy1.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SoulsLike/SoulsLikesTypes.h"
#include "AOEExplosion.h"
#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "CombatComponent.h"

ABossEnemy1::ABossEnemy1()
{
}

void ABossEnemy1::BeginPlay()
{
    Super::BeginPlay();
}

void ABossEnemy1::OnNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload &Payload)
{
    Super::OnNotifyBegin(NotifyName, Payload);
    if (NotifyName == "AOE")
    {
        SpawnAOE();
    }
}
void ABossEnemy1::OnNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload &Payload)
{
}

void ABossEnemy1::SpawnAOE()
{
    if (!AOEExplosionClass)
        return;

    const int32 NumExplosions = 3;    // 소환 개수
    const float SpawnRadius = 400.0f; // 랜덤 범위 (반지름)
    const float TimeInterval = 0.2f;  // 폭발 간 시간차 (초)

    FVector CenterLocation = FVector::ZeroVector;
    AAIController *AIC = Cast<AAIController>(GetController());

    if (AIC)
    {
        CenterLocation = AIC->GetFocalPoint();
    }
    else
    {
        CenterLocation = GetActorLocation() + (GetActorForwardVector() * 300.0f);
    }

    for (int32 i = 1; i <= NumExplosions; i++)
    {
        float Delay = i * TimeInterval;

        FTimerHandle TimerHandle;
        FTimerDelegate TimerDel;

        // 람다 함수 사용: this, 중심점, 반경, 클래스 정보를 캡처해서 나중에 실행함
        // BindWeakLambda를 써야 보스가 죽어서 사라진 뒤에 폭발이 터지려 할 때 크래시가 안 남
        TimerDel.BindWeakLambda(this, [this, CenterLocation, SpawnRadius]()
                                {
            if (!this->AOEExplosionClass) return;

            // 랜덤 위치 계산
            FVector RandomOffset;
            RandomOffset.X = FMath::RandRange(-SpawnRadius, SpawnRadius);
            RandomOffset.Y = FMath::RandRange(-SpawnRadius, SpawnRadius);
            RandomOffset.Z = 0.0f;

            FVector TargetFocalPoint = CenterLocation + RandomOffset;
            FVector FinalSpawnLocation = TargetFocalPoint;

            FHitResult HitResult;
            FVector TraceStart = TargetFocalPoint + FVector(0, 0, 500.0f);
            FVector TraceEnd = TargetFocalPoint - FVector(0, 0, 1000.0f);

            FCollisionQueryParams Params;
            Params.AddIgnoredActor(this);

            bool bHit = GetWorld()->LineTraceSingleByChannel(
                HitResult,
                TraceStart,
                TraceEnd,
                ECC_Visibility,
                Params);

            if (bHit)
            {
                FinalSpawnLocation = HitResult.Location;
            }
            else
            {
                FinalSpawnLocation.Z = GetActorLocation().Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
            }

            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;
            SpawnParams.Instigator = this;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            GetWorld()->SpawnActor<AAOEExplosion>(this->AOEExplosionClass, FinalSpawnLocation, FRotator::ZeroRotator, SpawnParams); });

        GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, Delay, false);
    }
}
