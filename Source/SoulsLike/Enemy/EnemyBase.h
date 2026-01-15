// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CombatComponent.h"
#include "EnemyBase.generated.h"

struct AttackTypeAttribute
{
	float MinimumDistance;	  // 공격 발동 최소 거리
	float WaitTime;			  // 대기 시간
	UAnimMontage *AttackAnim; // 재생 애니메이션
};
UCLASS()
class SOULSLIKE_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AEnemyBase();

	int32 MaxAttackType = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UCombatComponent *CombatComp;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	float Health = 100.0f;
	UPROPERTY(EditAnywhere)
	float Stamina = 100.0f;

	bool bIsDead = false;
	bool bIsStunned = false;

	bool bCanMove = false;
	bool bIsAttacking = false; // 공격 중인지
	bool bIsHitting = false;   // 피격 중인지
	bool bCanBeHit = true;	   // 피격 가능한지
	bool bIsSwept = false;	   // 스윕을 받는 중인지

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TArray<UAnimMontage *> AttackMontages;

	TArray<AttackTypeAttribute> AttackTypeAttributes;

	UFUNCTION()
	virtual void OnNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload &Payload);

	UFUNCTION()
	virtual void OnNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload &Payload);

	virtual bool PlayAttackMontage(int32 AttackType);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual float AddHealth(float Amount);
	virtual void Die();
	virtual void Stun();
	bool Attack(int32 AttackType);

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;
};
