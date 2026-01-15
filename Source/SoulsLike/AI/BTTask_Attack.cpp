// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_Attack.h"
#include "AIController.h"
#include "SoulsLike/Enemy/EnemyBase.h"
#include "SoulsLike/Enemy/CombatComponent.h"

UBTTask_Attack::UBTTask_Attack()
{
    NodeName = "Attack";
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory)
{
    Super::ExecuteTask(OwnerComp, NodeMemory);
    APawn *Pawn = OwnerComp.GetAIOwner()->GetPawn();
    if (!Pawn)
    {
        return EBTNodeResult::Failed;
    }
    AEnemyBase *Enemy = Cast<AEnemyBase>(Pawn);
    if (!Enemy)
    {
        return EBTNodeResult::Failed;
    }
    if (Enemy->Attack())
    {
        return EBTNodeResult::Succeeded;
    }
    UE_LOG(LogTemp, Error, TEXT("Attack Failed!"));
    return EBTNodeResult::Failed;
}