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
	virtual void BeginPlay() override;
	virtual bool PlayAttackMontage() override;
};
