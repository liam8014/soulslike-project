// Fill out your copyright notice in the Description page of Project Settings.

#include "AttackWindow.h"
#include "SoulsLike/Enemy/CombatComponent.h"
#include "SoulsLike/Enemy/EnemyBase.h"

void UAttackWindow::NotifyBegin(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, float TotalDuration, const FAnimNotifyEventReference &EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
    if (AEnemyBase *Enemy = Cast<AEnemyBase>(MeshComp->GetOwner()))
    {
        if (UCombatComponent *CombatComp = Enemy->CombatComp)
        {
            CombatComp->AttackBeforeTrace();
            CombatComp->SetAttackAttribute(DamageMultiplier, KnockBackDistance, AttackDirection);
            CombatComp->EnableAttackSweep();
        }
    }
}

void UAttackWindow::NotifyEnd(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, const FAnimNotifyEventReference &EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);
    if (AEnemyBase *Enemy = Cast<AEnemyBase>(MeshComp->GetOwner()))
    {
        if (UCombatComponent *CombatComp = Enemy->CombatComp)
        {
            CombatComp->DisableAttackSweep();
        }
    }
}