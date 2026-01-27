// Fill out your copyright notice in the Description page of Project Settings.

#include "BossHealthBar.h"
#include "Components/ProgressBar.h"
void UBossHealthBar::NativeConstruct()
{
    Super::NativeConstruct();
    if (HealthBar)
        HealthBar->SetPercent(1.0f);
    if (GhostBar)
        GhostBar->SetPercent(1.0f);
}

void UBossHealthBar::NativeTick(const FGeometry &MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // 잔상 바가 없거나, 이미 다 줄어들었으면 연산 안 함
    if (!GhostBar || FMath::IsNearlyEqual(CurrentGhostPercent, TargetPercent-0.001, 0.001f))
        return;

    // 1. 대기 시간 체크
    if (GhostDelayTimer > 0.0f)
    {
        GhostDelayTimer -= InDeltaTime;
    }
    else
    {
        // 2. 부드럽게 줄어들기 (Interp)
        CurrentGhostPercent = FMath::FInterpTo(CurrentGhostPercent, TargetPercent, InDeltaTime, INTERP_SPEED);

        GhostBar->SetPercent(CurrentGhostPercent);
    }
}

void UBossHealthBar::SetHealthPercent(float Percent)
{
    // 1. 입력값 안전장치 (0~1 사이로 고정)
    float NewPercent = FMath::Clamp(Percent, 0.0f, 1.0f);

    // 2. 메인 바(빨강)는 즉시 갱신 (기존 로직)
    if (HealthBar)
    {
        HealthBar->SetPercent(NewPercent);
    }

    // 3. 잔상 로직 분기
    if (NewPercent < CurrentGhostPercent)
    {
        // [데미지 입음]
        // 목표치만 갱신하고, 타이머를 켜서 잠시 멈추게 함
        TargetPercent = NewPercent;
        GhostDelayTimer = DELAY_TIME;
    }
    else
    {
        if (GhostBar)
            GhostBar->SetPercent(NewPercent);
        CurrentGhostPercent = NewPercent;
        TargetPercent = NewPercent;
    }
}