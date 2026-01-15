// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "BTTask_MoveToLocation.generated.h"

/**
 *
 */
UCLASS()
class SOULSLIKE_API UBTTask_MoveToLocation : public UBTTask_MoveTo
{
	GENERATED_BODY()

public:
	UBTTask_MoveToLocation();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "AI")
	FBlackboardKeySelector DynamicAcceptanceRadiusKey;
};
