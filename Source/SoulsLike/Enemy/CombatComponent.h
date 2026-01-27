// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulsLike/Character/PlayerCharacter.h"
#include "SoulsLike/SoulsLikesTypes.h"

#include "CombatComponent.generated.h"

// class UAnimMontage;
class AEnemyBase;
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SOULSLIKE_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCombatComponent();
	void EnableAttackSweep();
	void DisableAttackSweep();

	void SetAttackAttribute(float Multiplier, float KnockBack, EAttackDirection Direction, FName Socket);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	AEnemyBase *Owner;
	bool bIsSweeping = false;
	bool bIsAttacking = false;

	UPROPERTY(EditAnywhere)
	float AttackPower = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawDebugShape = true;

	UPROPERTY(EditAnywhere, Category = "Debug")
	int32 FixedAttackType = 0;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bShouldFixAttackType = false;

	float DamageMultiplier = 1.0f;
	float KnockBackDistance = 600.0f;

	EAttackDirection AttackDirection = EAttackDirection::AD_Forward;

	UPROPERTY(EditAnywhere)
	float AttackRadius = 100;

	UPROPERTY(EditAnywhere)
	float AttackRange = 70.0f;

	void AttackTrace();
	TSet<AActor *> ProcessedActors;

	UPROPERTY(EditAnywhere, Category = "VFX")
	class UParticleSystem *HitImpactVFX;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	void AttackBeforeTrace();
	void SetRandomAttackType();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TArray<FAttackTypeAttribute> AttackTypeAttributes;

	int32 NextAttackType = 0;
	float NextAcceptance = 170.0f;
	float NextWaitTime = 5.0f;

	FName TraceSocket = FName("weapon");
	UAnimMontage *NextAnimMontage = nullptr;
};
