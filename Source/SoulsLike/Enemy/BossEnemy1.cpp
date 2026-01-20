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

    FVector SpawnLocation = FVector::ZeroVector;
    FRotator SpawnRotation = FRotator::ZeroRotator;
    AAIController *AIC = Cast<AAIController>(GetController());

    if (AIC)
    {
        // 1. AI가 보고 있는 지점(Focus Location) 가져오기
        // Focus Actor가 있으면 그 액터의 위치를, 없으면 지정된 좌표를 리턴합니다.
        FVector FocalPoint = AIC->GetFocalPoint();

        // 2. 바닥 보정 (LineTrace)
        // FocalPoint는 공중(눈높이)일 수 있으므로, 그 위치에서 아래로 레이를 쏴서 바닥을 찾습니다.
        FHitResult HitResult;
        FVector TraceStart = FocalPoint + FVector(0, 0, 500.0f); // 위에서
        FVector TraceEnd = FocalPoint - FVector(0, 0, 1000.0f);  // 아래로

        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this); // 나는 무시

        bool bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            TraceStart,
            TraceEnd,
            ECC_Visibility, // 또는 ECC_WorldStatic
            Params);

        if (bHit)
        {
            // 바닥을 찾았으면 그 위치로 설정
            SpawnLocation = HitResult.Location;
        }
        else
        {
            // 바닥을 못 찾았으면(허공이라면) 그냥 FocalPoint 높이에서 Z만 살짝 조정하거나 그대로 사용
            SpawnLocation = FocalPoint;
            SpawnLocation.Z = GetActorLocation().Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
        }
    }
    else
    {
        // AI 컨트롤러가 없거나 실패 시 정면 사용
        SpawnLocation = GetActorLocation() + (GetActorForwardVector() * 300.0f);
        SpawnLocation.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    }

    // 3. 액터 소환
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = this;

    GetWorld()->SpawnActor<AAOEExplosion>(AOEExplosionClass, SpawnLocation, SpawnRotation, SpawnParams);
}
