// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CombatComponent.h"
#include "AttributeComponent.h"
#include "SoulsLike/SoulsLikesTypes.h"
#include "EnemyBase.generated.h"

struct FTimerHandle;
UCLASS()
class SOULSLIKE_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()
public:
	void PlayMeshJitter(float Intensity = 5.0f, float Duration = 0.15f);

protected:
	void HandleMeshJitter();

	void RestoreMeshPosition();

private:
	FTimerHandle JitterTimerHandle;
	FTimerHandle JitterRestoreTimerHandle;

	FVector OriginalMeshLocation;
	float CurrentJitterIntensity;

public:
	// Sets default values for this pawn's properties
	AEnemyBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UCombatComponent *CombatComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attribute")
	UAttributeComponent *AttributeComp;

	bool bCanMove = true;
	UPROPERTY(EditAnywhere)
	bool bIsSpawnable = false;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChangedDelegate OnHealthChanged;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool bIsDead = false;
	bool bIsStunned = false;

	bool bIsAttacking = false; // 공격 중인지
	bool bIsHitting = false;   // 피격 중인지
	bool bCanBeHit = true;	   // 피격 가능한지
	bool bIsSwept = false;	   // 스윕을 받는 중인지

	UFUNCTION()
	virtual void OnNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload &Payload);

	UFUNCTION()
	virtual void OnNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload &Payload);

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage *StunMontage;

	bool PlayAttackMontage();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Hit(float Damage, float StaminaDamage);
	virtual void Die();
	virtual void Stun();
	virtual bool Attack();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;
};
