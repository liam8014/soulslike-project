// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerAttributeComponent.h"
#include "SoulsLike/Character/PlayerDataAsset.h"
void UPlayerAttributeComponent::InitializeFromDataAsset(UPlayerDataAsset *DataAsset)
{
    PlayerData = DataAsset;
}

bool UPlayerAttributeComponent::TryConsumeStamina(float Amount)
{
    if (GetStamina() < Amount)
    {
        return false;
    }
    ChangeStamina(-Amount);
    return true;
}
float UPlayerAttributeComponent::GetMaxWalkSpeed() const
{
    return PlayerData ? PlayerData->MaxWalkSpeed : MaxWalkSpeed;
}

float UPlayerAttributeComponent::GetMaxSprintSpeed() const
{
    return PlayerData ? PlayerData->MaxSprintSpeed : MaxSprintSpeed;
}

float UPlayerAttributeComponent::GetAttackSweepRange() const
{
    return PlayerData ? PlayerData->AttackSweepRange : AttackSweepRange;
}

float UPlayerAttributeComponent::GetAttackSweepRadius() const
{
    return PlayerData ? PlayerData->AttackSweepRadius : AttackSweepRadius;
}

float UPlayerAttributeComponent::GetLightAttackStaminaCost() const
{
    return PlayerData ? PlayerData->LightAttackStaminaCost : LightAttackStaminaCost;
}

float UPlayerAttributeComponent::GetHeavyAttackStaminaCost() const
{
    return PlayerData ? PlayerData->HeavyAttackStaminaCost : HeavyAttackStaminaCost;
}

float UPlayerAttributeComponent::GetSprintActivationCost() const
{
    return PlayerData ? PlayerData->SprintActivationCost : SprintActivationCost;
}

float UPlayerAttributeComponent::GetSprintStaminaCost() const
{
    return PlayerData ? PlayerData->SprintStaminaCost : SprintStaminaCost;
}

float UPlayerAttributeComponent::GetDodgeStaminaCost() const
{
    return PlayerData ? PlayerData->DodgeStaminaCost : DodgeStaminaCost;
}

float UPlayerAttributeComponent::GetParryStaminaCost() const
{
    return PlayerData ? PlayerData->ParryStaminaCost : ParryStaminaCost;
}

float UPlayerAttributeComponent::GetParryStaminaReturn() const
{
    return PlayerData ? PlayerData->ParryStaminaReturn : ParryStaminaReturn;
}

float UPlayerAttributeComponent::GetGuardRate() const
{
    return PlayerData ? PlayerData->GuardRate : GuardRate;
}

float UPlayerAttributeComponent::GetLightAttackSpeed() const
{
    return PlayerData ? PlayerData->LightAttackSpeed : LightAttackSpeed;
}

float UPlayerAttributeComponent::GetGuardStaminaCostRate() const
{
    return PlayerData ? PlayerData->GuardStaminaCostRate : GuardStaminaCostRate;
}

float UPlayerAttributeComponent::GetGuardStaminaRegenMultiplier() const
{
    return PlayerData ? PlayerData->GuardStaminaRegenMultiplier : GuardStaminaRegenMultiplier;
}