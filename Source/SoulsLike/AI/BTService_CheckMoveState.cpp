// Fill out your copyright notice in the Description page of Project Settings.

#include "BTService_CheckMoveState.h"
#include "SoulsLike/Enemy/EnemyBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTService_CheckMoveState::UBTService_CheckMoveState()
{
    NodeName = "Update Move State";
}

void UBTService_CheckMoveState::TickNode(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    APawn *Pawn = OwnerComp.GetAIOwner()->GetPawn();
    AEnemyBase *Enemy = Cast<AEnemyBase>(Pawn);

    if (Enemy)
    {
        // Enemy의 변수 값을 블랙보드에 그대로 복사
        OwnerComp.GetBlackboardComponent()->SetValueAsBool(GetSelectedBlackboardKey(), Enemy->bCanMove);
    }
}
