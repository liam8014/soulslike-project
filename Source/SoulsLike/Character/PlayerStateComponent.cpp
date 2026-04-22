#include "PlayerStateComponent.h"

UPlayerStateComponent::UPlayerStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

EMovementState UPlayerStateComponent::GetMovementState() const
{
	return CurrentMovementState;
}

bool UPlayerStateComponent::IsInMovementState(EMovementState State) const
{
	return CurrentMovementState == State;
}

bool UPlayerStateComponent::CanMove() const
{
	return CurrentMovementState == EMovementState::MS_Idle ||
		   CurrentMovementState == EMovementState::MS_Moving;
}

bool UPlayerStateComponent::SetMovementState(EMovementState NewState)
{
	if (CurrentMovementState == NewState || !CanTransitionTo(NewState))
	{
		return false;
	}

	const EMovementState PreviousState = CurrentMovementState;
	CurrentMovementState = NewState;
	OnMovementStateChanged.Broadcast(PreviousState, CurrentMovementState);
	return true;
}

bool UPlayerStateComponent::CanTransitionTo(EMovementState NewState) const
{
	if (CurrentMovementState == EMovementState::MS_Dead)
	{
		return NewState == EMovementState::MS_Dead;
	}

	return true;
}
