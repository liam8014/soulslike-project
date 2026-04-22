#include "PlayerUIComponent.h"

#include "Blueprint/UserWidget.h"
#include "SoulsLike/UI/StatusBar.h"

UPlayerUIComponent::UPlayerUIComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerUIComponent::InitializeUI(APlayerController* PlayerController, TSubclassOf<UStatusBar> InStatusBarClass, TSubclassOf<UUserWidget> InFocusIndicatorWidgetClass)
{
	if (InStatusBarClass && !StatusBar)
	{
		StatusBar = CreateWidget<UStatusBar>(GetWorld(), InStatusBarClass);
		if (StatusBar)
		{
			StatusBar->AddToViewport();
		}
	}

	if (PlayerController && InFocusIndicatorWidgetClass && !FocusIndicatorWidget)
	{
		FocusIndicatorWidget = CreateWidget<UUserWidget>(PlayerController, InFocusIndicatorWidgetClass);
		if (FocusIndicatorWidget)
		{
			FocusIndicatorWidget->AddToViewport();
			FocusIndicatorWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UPlayerUIComponent::SetStatusBarVisible(bool bVisible) const
{
	if (StatusBar)
	{
		StatusBar->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void UPlayerUIComponent::SetFocusIndicatorVisible(bool bVisible) const
{
	if (FocusIndicatorWidget)
	{
		FocusIndicatorWidget->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void UPlayerUIComponent::SetFocusIndicatorScreenPosition(const FVector2D& ScreenPosition) const
{
	if (FocusIndicatorWidget)
	{
		FocusIndicatorWidget->SetPositionInViewport(ScreenPosition, false);
	}
}

void UPlayerUIComponent::SetHealthPercent(float Percent) const
{
	if (StatusBar)
	{
		StatusBar->SetHealthPercent(Percent);
	}
}

void UPlayerUIComponent::SetStaminaPercent(float Percent) const
{
	if (StatusBar)
	{
		StatusBar->SetStaminaPercent(Percent);
	}
}

bool UPlayerUIComponent::HasFocusIndicator() const
{
	return FocusIndicatorWidget != nullptr;
}
