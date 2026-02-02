// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/Soundbase.h"
#include "AOEExplosion.generated.h"

class UParticleSystem;
UCLASS()
class SOULSLIKE_API AAOEExplosion : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAOEExplosion();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	UParticleSystem *BeforeEffect1;
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	UParticleSystem *BeforeEffect2;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UParticleSystem *ExplosionEffect;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackRadius = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ExplosionDelay = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float TraceDelay = 0.4f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float DamageAmount = 20.0f;
	UPROPERTY(EditDefaultsOnly, Category = "SFX")
	USoundBase *ExplosionSound;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// 실제 폭발 로직 함수
	void Explode();
	void AttackTrace();
};
