// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SoulsLike/Component/AttributeComponent.h"
#include "PlayerAttributeComponent.generated.h"

/**
 *
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SOULSLIKE_API UPlayerAttributeComponent : public UAttributeComponent
{
	GENERATED_BODY()
public:
	void InitializeFromDataAsset(class UPlayerDataAsset *DataAsset);

	bool TryConsumeStamina(float Amount);

	// Getters (선언만 유지)
	float GetMaxWalkSpeed() const;
	float GetMaxSprintSpeed() const;
	float GetAttackSweepRange() const;
	float GetAttackSweepRadius() const;
	float GetLightAttackStaminaCost() const;
	float GetHeavyAttackStaminaCost() const;
	float GetSprintActivationCost() const;
	float GetSprintStaminaCost() const;
	float GetDodgeStaminaCost() const;
	float GetParryStaminaCost() const;
	float GetParryStaminaReturn() const;
	float GetGuardRate() const;
	float GetLightAttackSpeed() const;
	float GetGuardStaminaCostRate() const;
	float GetGuardStaminaRegenMultiplier() const;

private:
	UPROPERTY()
	class UPlayerDataAsset *PlayerData;

	UPROPERTY(VisibleAnywhere, Category = "Locomotion")
	float MaxWalkSpeed = 450; // 최대 걷기 속도

	UPROPERTY(VisibleAnywhere, Category = "Locomotion")
	float MaxSprintSpeed = 1100; // 최대 달리기 속도

	UPROPERTY(VisibleAnywhere, Category = "Combat|Sweep")
	float AttackSweepRange = 120.0f; // 공격 판정 범위

	UPROPERTY(VisibleAnywhere, Category = "Combat|Sweep")
	float AttackSweepRadius = 5.0f; // 공격 판정 두께

	UPROPERTY(VisibleAnywhere, Category = "Combat|Stamina")
	float LightAttackStaminaCost = 6.0f; // 기본 공격 소모량

	UPROPERTY(VisibleAnywhere, Category = "Combat|Stamina")
	float HeavyAttackStaminaCost = 13.0f; // 강 공격 소모량

	UPROPERTY(VisibleAnywhere, Category = "Combat|Stamina")
	float DodgeStaminaCost = 25.0f; // 회피 소모량

	UPROPERTY(VisibleAnywhere, Category = "Combat|Stamina")
	float ParryStaminaCost = 20.0f; // 패링 소모량

	UPROPERTY(VisibleAnywhere, Category = "Combat|Stamina")
	float ParryStaminaReturn = 12.0f; // 패링 성공 시 회복량

	UPROPERTY(VisibleAnywhere, Category = "Combat")
	float GuardRate = 0.60f; // 가드율

	UPROPERTY(EditAnywhere, Category = "Combat")
	float LightAttackSpeed = 1.0f; // 일반 공격 속도

	UPROPERTY(VisibleAnywhere, Category = "Combat|Stamina")
	float GuardStaminaCostRate = 1.75f; // 피해량 대비 소모율

	UPROPERTY(VisibleAnywhere, Category = "Combat|Stamina")
	float GuardStaminaRegenMultiplier = 0.5f; // 가드 시 스테미나 회복율

	UPROPERTY(VisibleAnywhere, Category = "Movement|Stamina")
	float SprintActivationCost = 2.5f; // 달리기 시작 소모량

	UPROPERTY(VisibleAnywhere, Category = "Movement|Stamina")
	float SprintStaminaCost = 0.08f; // 달리기 유지 소모량
};
