// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "BossEnemy1.generated.h"

/**
 *
 */

class AAOEExplosion;
UCLASS()
class SOULSLIKE_API ABossEnemy1 : public AEnemyBase
{
	GENERATED_BODY()
public:
	ABossEnemy1();

	UPROPERTY(EditAnywhere, Category = "Pooling")
	bool bUsePooling = false;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TSubclassOf<AAOEExplosion> AOEExplosionClass;
	UPROPERTY(EditAnywhere, Category = "Combat")
	int32 NumExplosions = 3; // 소환 개수

	UPROPERTY(EditAnywhere, Category = "Combat")
	float TimeInterval = 0.1f; // 폭발 간 시간차 (초)

	void OnNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload &Payload) override;
	void OnNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload &Payload) override;

	void SpawnAOE();
};
