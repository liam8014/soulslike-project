// BTTask_MoveToLocation.cpp

#include "BTTask_MoveToLocation.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"

UBTTask_MoveToLocation::UBTTask_MoveToLocation()
{
    NodeName = TEXT("Move To (Dynamic Range)");

    // 이 키에는 Float 타입만 연결할 수 있도록 필터링
    DynamicAcceptanceRadiusKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_MoveToLocation, DynamicAcceptanceRadiusKey));
}

EBTNodeResult::Type UBTTask_MoveToLocation::ExecuteTask(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory)
{
    UBlackboardComponent *BlackboardComp = OwnerComp.GetBlackboardComponent();

    if (BlackboardComp)
    {
        float DynamicRadius = BlackboardComp->GetValueAsFloat(DynamicAcceptanceRadiusKey.SelectedKeyName);
        if (DynamicRadius > 0.0f)
        {
            AcceptableRadius = DynamicRadius;
        }
    }
    return Super::ExecuteTask(OwnerComp, NodeMemory);
}