// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_ApplyAttackTypeAttribute.generated.h"

/**
 *
 */
UCLASS()
class SOULSLIKE_API UBTTask_ApplyAttackTypeAttribute : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
public:
	UBTTask_ApplyAttackTypeAttribute();

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector WaitTime;
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory) override;
};
