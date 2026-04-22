// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SoulsLike/SoulsLikesTypes.h"
#include "EnemyDataAsset.generated.h"

/**
 *
 */
UCLASS()
class SOULSLIKE_API UEnemyDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/* 어트리뷰트 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	float MaxStamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	float StaminaRegenAmount = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackPower = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Sweep")
	float AttackRadius = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Sweep")
	float AttackRange = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TArray<FAttackTypeAttribute> AttackPatterns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
	TObjectPtr<UParticleSystem> AttackImpactVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
	TObjectPtr<USoundBase> HitSFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
	TObjectPtr<UAnimMontage> DieMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
	TObjectPtr<UAnimMontage> StunMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	float DissolveSpeed = 0.5f;
};
