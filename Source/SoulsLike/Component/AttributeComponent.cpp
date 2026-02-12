// Fill out your copyright notice in the Description page of Project Settings.

#include "AttributeComponent.h"
#include "SoulsLike/Enemy/EnemyBase.h"

// Sets default values for this component's properties
UAttributeComponent::UAttributeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UAttributeComponent::InitAttribute(float Health, float Stamina, float StaminaRegen)
{
	MaxHealth = Health;
	SetHealth(MaxHealth);
	MaxStamina = Stamina;
	SetStamina(MaxStamina);
	StaminaRegenAmount = StaminaRegen;
}

float UAttributeComponent::GetHealth()
{
	return CurrentHealth;
}

float UAttributeComponent::GetStamina()
{
	return CurrentStamina;
}

void UAttributeComponent::SetHealth(float NewHealth)
{
	CurrentHealth = FMath::Clamp(NewHealth, 0.0f, MaxHealth);
	if (OnHealthChanged.IsBound())
	{
		OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	}
}

void UAttributeComponent::SetStamina(float NewStamina)
{
	CurrentStamina = FMath::Clamp(NewStamina, 0.0f, MaxStamina);
	if (OnStaminaChanged.IsBound())
	{
		OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
	}
}

void UAttributeComponent::SetStaminaRegenMultiplier(float NewRegenMultiplier)
{
	StaminaRegenMultiplier = NewRegenMultiplier;
}

float UAttributeComponent::GetStaminaRegenAmount()
{
	return StaminaRegenAmount;
}

float UAttributeComponent::GetBaseAttackPower()
{
	return BaseAttackPower;
}

void UAttributeComponent::ChangeHealth(float Amount)
{
	if (bCanChangeHealth)
	{
		SetHealth(FMath::Clamp(CurrentHealth + Amount, 0.0f, MaxHealth));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("You can't change health now."));
	}
}

void UAttributeComponent::ChangeStamina(float Amount)
{
	if (bCanChangeStamina)
	{
		SetStamina(FMath::Clamp(CurrentStamina + Amount, 0.0f, MaxStamina));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("You can't change stamina now."));
	}
}

// Called when the game starts
void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();
	Owner = Cast<ACharacter>(GetOwner());
	if (Owner == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to set Owner"));
		return;
	}
	SetHealth(MaxHealth);
	SetStamina(MaxStamina);
	// ...
}

// Called every frame
void UAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bCanRegenStamina && CurrentStamina < MaxStamina)
	{
		ChangeStamina(StaminaRegenAmount * StaminaRegenMultiplier);
	}
}

void UAttributeComponent::EnableChangeHeatlh()
{
	bCanChangeHealth = true;
}

void UAttributeComponent::DisableChangeHeatlh()
{
	bCanChangeHealth = false;
}

void UAttributeComponent::EnableChangeStamina()
{
	bCanChangeStamina = true;
}

void UAttributeComponent::DisableChangeStamina()
{
	bCanChangeStamina = false;
}

void UAttributeComponent::EnableRegenStamina()
{
	bCanRegenStamina = true;
}

void UAttributeComponent::DisableRegenStamina()
{
	bCanRegenStamina = false;
}
