// Fill out your copyright notice in the Description page of Project Settings.

#include "AttributeComponent.h"
#include "EnemyBase.h"

// Sets default values for this component's properties
UAttributeComponent::UAttributeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UAttributeComponent::ChangeHealth(float Amount)
{
	if (bCanChangeHealth)
	{
		CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.0f, MaxHealth);
	}
}

void UAttributeComponent::ChangeStamina(float Amount)
{
	if (bCanChangeStamina)
	{
		CurrentStamina = FMath::Clamp(CurrentStamina + Amount, 0.0f, MaxStamina);
		UE_LOG(LogTemp, Warning, TEXT("Current Stamina : %f"), CurrentStamina);
		if (CurrentStamina == 0)
		{
			Owner->Stun();
			CurrentStamina = MaxStamina;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("You can't change stamina now."));
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

void UAttributeComponent::DisableChangeStaimna()
{
	bCanChangeStamina = false;
}

// Called when the game starts
void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();
	Owner = Cast<AEnemyBase>(GetOwner());
	if (Owner == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to set Owner"));
	}

	CurrentHealth = MaxHealth;
	CurrentStamina = MaxStamina;
	// ...
}

// Called every frame
void UAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentStamina < MaxStamina)
	{
		CurrentStamina = FMath::Clamp(CurrentStamina + StaminaRegenAmount, CurrentStamina, MaxStamina);
	}
}
