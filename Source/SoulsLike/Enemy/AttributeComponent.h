// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeComponent.generated.h"
class AEnemyBase;
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SOULSLIKE_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UAttributeComponent();
	float GetHealth();
	float GetStamina();

	void ChangeHealth(float Amount);
	void ChangeStamina(float Amount);

	void EnableChangeHeatlh();
	void DisableChangeHeatlh();

	void EnableChangeStamina();
	void DisableChangeStaimna();

	AEnemyBase *Owner;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Attributes")
	float MaxHealth = 100.0f;
	UPROPERTY(EditAnywhere, Category = "Attributes")
	float MaxStamina = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Attributes")
	float StaminaRegenAmount = 10.0f;

	float CurrentHealth;
	float CurrentStamina;

	bool bCanChangeHealth = true;
	bool bCanChangeStamina = true;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
};
