// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/Soundbase.h"
#include "SoulsLike/ObjectPooling/PoolableInterface.h"
#include "AOEExplosion.generated.h"

class UParticleSystem;
class UPoolManagerSubsystem;
UCLASS()
class SOULSLIKE_API AAOEExplosion : public AActor,
									public IPoolableInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAOEExplosion();

	virtual void OnSpawnFromPool() override;
	virtual void OnReturnToPool() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Pooling")
	bool bUsePooling = false;

	UPROPERTY()
	TObjectPtr<UPoolManagerSubsystem> PoolMgr;

	UPROPERTY(EditDefaultsOnly, Category = "Pooling")
	TSubclassOf<AActor> BeforeEffect1Class;
	UPROPERTY(EditDefaultsOnly, Category = "Pooling")
	TSubclassOf<AActor> BeforeEffect2Class;
	UPROPERTY(EditDefaultsOnly, Category = "Pooling")
	TSubclassOf<AActor> ExplosionEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	TObjectPtr<UParticleSystem> BeforeEffect1;
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	TObjectPtr<UParticleSystem> BeforeEffect2;
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TObjectPtr<UParticleSystem> ExplosionEffect;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackRadius = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ExplosionDelay = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float TraceDelay = 0.4f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float DamageAmount = 20.0f;
	UPROPERTY(EditDefaultsOnly, Category = "SFX")
	TObjectPtr<USoundBase> ExplosionSound;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// 실제 폭발 로직 함수
	void Explode();
	void AttackTrace();
};
