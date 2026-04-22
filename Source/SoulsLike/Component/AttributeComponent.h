// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeComponent.generated.h"
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, CurrentHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaChanged, float, CurrentStamina, float, MaxStamina);

class AEnemyBase;
class UBossHealthBar;
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SOULSLIKE_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UAttributeComponent();
	void InitAttribute(float Health, float Stamina, float StaminaRegen);

	float GetHealth();
	void SetHealth(float NewHealth);

	float GetStamina();
	void SetStamina(float NewStamina);

	void SetStaminaRegenMultiplier(float NewRegenMultiplier);
	float GetStaminaRegenAmount();

	float GetBaseAttackPower();

	void ChangeHealth(float Amount);
	void ChangeStamina(float Amount);

	void EnableChangeHeatlh();
	void DisableChangeHeatlh();

	void EnableChangeStamina();
	void DisableChangeStamina();

	void EnableRegenStamina();
	void DisableRegenStamina();

	TObjectPtr<ACharacter> Owner;

	FOnHealthChanged OnHealthChanged;
	FOnStaminaChanged OnStaminaChanged;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	float StaminaRegenMultiplier = 1.0f;

	UPROPERTY(VisibleAnywhere, Category = "Attributes")
	float StaminaRegenAmount = 0.2f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes")
	float BaseAttackPower = 10.0f;

	float CurrentHealth;
	float CurrentStamina;

	bool bCanChangeHealth = true;
	bool bCanChangeStamina = true;
	bool bCanRegenStamina = true;

public:
	UPROPERTY(VisibleAnywhere, Category = "Attributes")
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, Category = "Attributes")
	float MaxStamina = 100.0f;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
};
