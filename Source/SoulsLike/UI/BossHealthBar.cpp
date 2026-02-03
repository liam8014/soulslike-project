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

    if (!GhostBar || FMath::IsNearlyEqual(CurrentGhostPercent, TargetPercent-0.001, 0.001f))
        return;

    if (GhostDelayTimer > 0.0f)
    {
        GhostDelayTimer -= InDeltaTime;
    }
    else
    {
        CurrentGhostPercent = FMath::FInterpTo(CurrentGhostPercent, TargetPercent, InDeltaTime, INTERP_SPEED);

        GhostBar->SetPercent(CurrentGhostPercent);
    }
}

void UBossHealthBar::SetHealthPercent(float Percent)
{
    float NewPercent = FMath::Clamp(Percent, 0.0f, 1.0f);

    if (HealthBar)
    {
        HealthBar->SetPercent(NewPercent);
    }
    if (NewPercent < CurrentGhostPercent)
    {
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