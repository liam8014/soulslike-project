// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "SoulsLike/SoulsLikesTypes.h"
#include "AttackWindow.generated.h"

/**
 *
 */
UCLASS()
class SOULSLIKE_API UAttackWindow : public UAnimNotifyState
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, Category = "Combat Data")
	EAttackDirection AttackDirection = EAttackDirection::AD_Forward;

	UPROPERTY(EditAnywhere, Category = "Combat Data")
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Combat Data")
	float KnockBackDistance = 1000.0f;

	virtual void NotifyBegin(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, float TotalDuration, const FAnimNotifyEventReference &EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, const FAnimNotifyEventReference &EventReference) override;
};
