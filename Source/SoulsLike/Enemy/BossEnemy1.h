// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "BossEnemy1.generated.h"

/**
 *
 */
UCLASS()
class SOULSLIKE_API ABossEnemy1 : public AEnemyBase
{
	GENERATED_BODY()
public:
	ABossEnemy1();

protected:
	virtual bool Attack(int32 AttackIndex) override;
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload &Payload);

	UFUNCTION()
	void OnNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload &Payload);
};
