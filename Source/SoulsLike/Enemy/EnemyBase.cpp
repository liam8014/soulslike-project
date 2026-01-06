// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyBase.h"
// Sets default values
AEnemyBase::AEnemyBase()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AEnemyBase::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

float AEnemyBase::AddHealth(float Amount)
{
	const float Damage = FMath::Clamp(Amount, 0.f, Health);
	Health -= Damage;
	if (Health <= 0.f)
	{
		Die();
	}
	return Damage;
}
void AEnemyBase::Die()
{
}
void AEnemyBase::Stun()
{
}

bool AEnemyBase::Attack(int32 AttackIndex)
{
	if (!AttackMontages.IsValidIndex(AttackIndex) || AttackMontages[AttackIndex] == nullptr)
	{
		return false;
	}
	UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance || AnimInstance->Montage_IsPlaying(AttackMontages[AttackIndex]))
	{
		return false;
	}
	return true;
}