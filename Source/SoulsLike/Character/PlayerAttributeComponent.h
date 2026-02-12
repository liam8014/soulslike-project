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
	bool TryConsumeStamina(float Amount);

	FORCEINLINE float GetLightAttackStaminaCost() const { return LightAttackStaminaCost; }

	FORCEINLINE float GetHeavyAttackStaminaCost() const { return HeavyAttackStaminaCost; }

	FORCEINLINE float GetSprintActivationCost() const { return SprintActivationCost; }

	FORCEINLINE float GetSprintStaminaCost() const { return SprintStaminaCost; }

	FORCEINLINE float GetDodgeStaminaCost() const { return DodgeStaminaCost; }

	FORCEINLINE float GetParryStaminaCost() const { return ParryStaminaCost; }

	FORCEINLINE float GetParryStaminaReturn() const { return ParryStaminaReturn; }

	FORCEINLINE float GetGuardRate() const { return GuardRate; }

	FORCEINLINE float GetGuardStaminaCostRate() const { return GuardStaminaCostRate; }

	FORCEINLINE float GetGuardStaminaRegenMultiplier() const { return GuardStaminaRegenMultiplier; }

private:
	UPROPERTY(VisibleAnywhere, Category = "Attributes")
	float LightAttackStaminaCost = 6.0f; // 기본 공격 소모량

	UPROPERTY(VisibleAnywhere, Category = "Attributes")
	float HeavyAttackStaminaCost = 13.0f; // 강 공격 소모량

	UPROPERTY(VisibleAnywhere, Category = "Attributes")
	float SprintActivationCost = 2.5f; // 달리기 시작 소모량

	UPROPERTY(VisibleAnywhere, Category = "Attributes")
	float SprintStaminaCost = 0.08f; // 달리기 소모량

	UPROPERTY(VisibleAnywhere, Category = "Attributes")
	float DodgeStaminaCost = 25.0f; // 회피 소모량

	UPROPERTY(VisibleAnywhere, Category = "Attributes")
	float ParryStaminaCost = 20.0f; // 패링 소모량

	UPROPERTY(VisibleAnywhere, Category = "Attributes")
	float ParryStaminaReturn = 12.0f; // 패링 성공 시 회복량

	UPROPERTY(VisibleAnywhere, Category = "Attributes")
	float GuardRate = 0.60f; // 가드율

	UPROPERTY(VisibleAnywhere, Category = "Attributes")
	float GuardStaminaCostRate = 1.75f; // 피해량 대비 소모율

	UPROPERTY(VisibleAnywhere, Category = "Attributes")
	float GuardStaminaRegenMultiplier = 0.5f; // 가드 시 스테미나 회복율
};
