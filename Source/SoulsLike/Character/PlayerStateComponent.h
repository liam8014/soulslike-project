#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulsLike/SoulsLikesTypes.h"
#include "PlayerStateComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMovementStateChanged, EMovementState, PreviousState, EMovementState, NewState);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SOULSLIKE_API UPlayerStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerStateComponent();

	UFUNCTION(BlueprintCallable, Category = "State")
	EMovementState GetMovementState() const;

	UFUNCTION(BlueprintCallable, Category = "State")
	bool IsInMovementState(EMovementState State) const;

	UFUNCTION(BlueprintCallable, Category = "State")
	bool CanMove() const;

	UFUNCTION(BlueprintCallable, Category = "State")
	bool SetMovementState(EMovementState NewState);

	UPROPERTY(BlueprintAssignable, Category = "State")
	FOnMovementStateChanged OnMovementStateChanged;

private:
	bool CanTransitionTo(EMovementState NewState) const;

	UPROPERTY(VisibleAnywhere, Category = "State")
	EMovementState CurrentMovementState = EMovementState::MS_Idle;
};
