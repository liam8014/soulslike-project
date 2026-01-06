// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_MoveToLocation.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
UBTTask_MoveToLocation::UBTTask_MoveToLocation()
{
    NodeName = TEXT("Move To Location");
}
EBTNodeResult::Type UBTTask_MoveToLocation::ExecuteTask(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory)
{
    Super::ExecuteTask(OwnerComp, NodeMemory);
    AAIController *AIController = OwnerComp.GetAIOwner();
    if (nullptr == AIController)
    {
        return EBTNodeResult::Failed;
    }

    UBlackboardComponent *BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (nullptr == BlackboardComp)
    {
        return EBTNodeResult::Failed;
    }

    FVector TargetLocation = BlackboardComp->GetValueAsVector(GetSelectedBlackboardKey());

    AIController->MoveToLocation(TargetLocation, AcceptanceRadious);
    return EBTNodeResult::Succeeded;
}