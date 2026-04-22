// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BossHealthBar.generated.h"

/**
 *
 */
class UProgressBar;
UCLASS()
class SOULSLIKE_API UBossHealthBar : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void SetHealthPercent(float Percent);

protected:
	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry &MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> GhostBar;

private:
	float CurrentGhostPercent = 1.0f; // 현재 잔상의 위치
	float TargetPercent = 1.0f;		  // 실제 체력

	float GhostDelayTimer = 0.0f; // 멈춰있는 시간

	const float DELAY_TIME = 1.0f;	 // 피격 후 대기 시간 (1초)
	const float INTERP_SPEED = 5.0f; // 줄어드는 속도
};
