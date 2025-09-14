// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StatusBar.generated.h"

class UProgressBar;

/**
 *
 */
UCLASS()
class SOULSLIKE_API UStatusBar : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION()
	void SetHealthPercent(float Percent);
	void SetStaminaPercent(float Percent);

protected:
	virtual void NativeConstruct() override;
	UPROPERTY(meta = (BindWidget))
	UProgressBar *HealthBar;

	UPROPERTY(meta = (BindWidget))
	UProgressBar *StaminaBar;
};
