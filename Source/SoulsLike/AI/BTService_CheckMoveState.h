// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_CheckMoveState.generated.h"

/**
 *
 */
UCLASS()
class SOULSLIKE_API UBTService_CheckMoveState : public UBTService_BlackboardBase
{
	GENERATED_BODY()
public:
	UBTService_CheckMoveState();

protected:
	virtual void TickNode(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory, float DeltaSeconds) override;
};
