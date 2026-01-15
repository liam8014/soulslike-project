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
    if (UAnimInstance *AnimInst = GetMesh()->GetAnimInstance())
    {
        AnimInst->OnPlayMontageNotifyBegin.AddDynamic(this, &ABossEnemy1::OnNotifyBegin); // NotifyState 시작(NotifyBegin) 바인딩
        AnimInst->OnPlayMontageNotifyEnd.AddDynamic(this, &ABossEnemy1::OnNotifyEnd);     // NotifyState 종료(NotifyEnd) 바인딩
        UE_LOG(LogTemp, Display, TEXT("AnimInst is Set"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("No AnimInst!"));
    }
    AttackTypeAttribute Swing = {
        170,
        3.0,
        AttackMontages[0]};
    MaxAttackType = AttackTypeAttributes.Num();
}
bool ABossEnemy1::PlayAttackMontage(int32 AttackType)
{
    if (!Super::PlayAttackMontage(AttackType))
    {
        return false;
    }
    switch (AttackType)
    {
    case 0:
        CombatComp->SetAttackAttribute(1.0, 200, EAttackDirection::AD_Left);
        PlayAnimMontage(AttackMontages[0]);
        break;
    case 2:
        CombatComp->SetAttackAttribute(2.0, 1200, EAttackDirection::AD_Forward);
        PlayAnimMontage(AttackMontages[2]);
        break;
    case 3:
        CombatComp->SetAttackAttribute(2.0, 1200, EAttackDirection::AD_Forward);
        PlayAnimMontage(AttackMontages[3]);
        break;
    default:
        return false;
    }

    return true;
}
void ABossEnemy1::OnNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload &Payload)
{
    Super::OnNotifyBegin(NotifyName, Payload);
    if (NotifyName == TEXT("Swing2"))
    {
        CombatComp->SetAttackAttribute(1.0, 3000, EAttackDirection::AD_Right);
        PlayAnimMontage(AttackMontages[1]);
    }
}

void ABossEnemy1::OnNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload &Payload)
{
    Super::OnNotifyEnd(NotifyName, Payload);
}
