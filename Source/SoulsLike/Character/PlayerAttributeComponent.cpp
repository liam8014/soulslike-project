// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerAttributeComponent.h"

bool UPlayerAttributeComponent::TryConsumeStamina(float Amount)
{
    if (GetStamina() < Amount)
    {
        return false;
    }
    ChangeStamina(-Amount);
    return true;
}
