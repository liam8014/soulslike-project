// Fill out your copyright notice in the Description page of Project Settings.

#include "AttributeComponent.h"
#include "SoulsLike/UI/BossHealthBar.h"
#include "EnemyBase.h"

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
	CurrentHealth = MaxHealth;
	MaxStamina = Stamina;
	CurrentStamina = MaxStamina;
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
}

void UAttributeComponent::SetStamina(float NewStamina)
{
	CurrentStamina = FMath::Clamp(NewStamina, 0.0f, MaxStamina);
}

void UAttributeComponent::ChangeHealth(float Amount)
{
	if (bCanChangeHealth)
	{
		CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.0f, MaxHealth);
		BossHealthBar->SetHealthPercent(CurrentHealth / MaxHealth);
		if (CurrentHealth <= 0)
		{
			Owner->Die();
		}
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

void UAttributeComponent::HideHealthBar()
{
	if (BossHealthBar)
	{
		BossHealthBar->RemoveFromParent();
	}
}

// Called when the game starts
void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();
	Owner = Cast<AEnemyBase>(GetOwner());
	if (Owner == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to set Owner"));
		return;
	}

	CurrentHealth = MaxHealth;
	CurrentStamina = MaxStamina;

	if (BossHealthBarClass && Owner && !Owner->bIsSpawnable)
	{
		BossHealthBar = CreateWidget<UBossHealthBar>(GetWorld(), BossHealthBarClass);
		if (BossHealthBar)
		{
			BossHealthBar->AddToViewport();
			BossHealthBar->SetHealthPercent(CurrentHealth / MaxHealth);
		}
	}
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