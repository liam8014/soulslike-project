// Fill out your copyright notice in the Description page of Project Settings.

#include "StatusBar.h"
#include "Components/ProgressBar.h"

void UStatusBar::NativeConstruct()
{
    Super::NativeConstruct();
}
void UStatusBar::SetHealthPercent(float Percent)
{
    if (HealthBar)
    {
        HealthBar->SetPercent(Percent);
    }
}
void UStatusBar::SetStaminaPercent(float Percent)
{
    if (StaminaBar)
    {
        StaminaBar->SetPercent(Percent);
    }
}