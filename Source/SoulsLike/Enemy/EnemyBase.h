// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyBase.generated.h"

UCLASS()
class SOULSLIKE_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AEnemyBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	float Health = 100.0f;
	float Stamina = 100.0f;
	float AttackPower = 10.0f;
	float MoveSpeed = 300.0f;

	bool bIsDead = false;
	bool bIsStunned = false;

	bool bCanMove = false;
	bool bIsAttacking = false; // 공격 중인지
	bool bIsHitting = false;   // 피격 중인지
	bool bCanBeHit = true;	   // 피격 가능한지
	bool bIsSwept = false;	   // 스윕을 받는 중인지

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TArray<UAnimMontage *> AttackMontages;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual float AddHealth(float Amount);
	virtual void Die();
	virtual void Stun();
	virtual bool Attack(int32 AttackIndex);

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;
};
