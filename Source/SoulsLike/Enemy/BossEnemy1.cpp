// Fill out your copyright notice in the Description page of Project Settings.

#include "BossEnemy1.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SoulsLike/SoulsLikesTypes.h"
#include "CombatComponent.h"

ABossEnemy1::ABossEnemy1()
{
}

void ABossEnemy1::BeginPlay()
{
    Super::BeginPlay();
}
bool ABossEnemy1::PlayAttackMontage()
{
    if (!Super::PlayAttackMontage())
    {
        return false;
    }
    switch (CombatComp->NextAttackType)
    {
    case 0:
        CombatComp->SetAttackAttribute(1.0, 200, EAttackDirection::AD_Left);
        break;
    case 1:
        CombatComp->SetAttackAttribute(2.0, 1200, EAttackDirection::AD_Forward);
        break;
    case 2:
        CombatComp->SetAttackAttribute(2.0, 1200, EAttackDirection::AD_Forward);
        break;
    default:
        return false;
    }
    PlayAnimMontage(CombatComp->NextAnimMontage);
    return true;
}
