#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerUIComponent.generated.h"

class APlayerController;
class UStatusBar;
class UUserWidget;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SOULSLIKE_API UPlayerUIComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerUIComponent();

	void InitializeUI(APlayerController* PlayerController, TSubclassOf<UStatusBar> InStatusBarClass, TSubclassOf<UUserWidget> InFocusIndicatorWidgetClass);
	void SetStatusBarVisible(bool bVisible) const;
	void SetFocusIndicatorVisible(bool bVisible) const;
	void SetFocusIndicatorScreenPosition(const FVector2D& ScreenPosition) const;
	void SetHealthPercent(float Percent) const;
	void SetStaminaPercent(float Percent) const;
	bool HasFocusIndicator() const;

private:
	UPROPERTY()
	TObjectPtr<UStatusBar> StatusBar;

	UPROPERTY()
	TObjectPtr<UUserWidget> FocusIndicatorWidget;
};
