// Fill out your copyright notice in the Description page of Project Settings.
#include "BTTask_ApplyAttackTypeAttribute.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SoulsLike/Enemy/EnemyBase.h"
#include "AIController.h"

UBTTask_ApplyAttackTypeAttribute::UBTTask_ApplyAttackTypeAttribute()
{
    NodeName = TEXT("Apply Attack Type Attribute");
}

EBTNodeResult::Type UBTTask_ApplyAttackTypeAttribute::ExecuteTask(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory)
{
    AEnemyBase *Enemy = Cast<AEnemyBase>(OwnerComp.GetAIOwner()->GetPawn());
    if (!Enemy)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to set Enemy"));
        return EBTNodeResult::Failed;
    }
    OwnerComp.GetBlackboardComponent()->SetValueAsFloat(GetSelectedBlackboardKey(), Enemy->CombatComp->NextAcceptance);
    OwnerComp.GetBlackboardComponent()->SetValueAsFloat(WaitTime.SelectedKeyName, Enemy->CombatComp->NextWaitTime);
    UE_LOG(LogTemp, Display, TEXT("NextAcceptance is %f, Next Wait Time is %f"), Enemy->CombatComp->NextAcceptance, Enemy->CombatComp->NextWaitTime);
    return EBTNodeResult::Succeeded;
}
