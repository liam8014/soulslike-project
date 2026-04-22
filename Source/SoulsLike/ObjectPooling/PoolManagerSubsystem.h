// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PoolManagerSubsystem.generated.h"

USTRUCT()
struct FPoolArray
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<AActor *> InactiveActors;

	int32 ActiveCount = 0;
};

UCLASS()
class SOULSLIKE_API UPoolManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	// 풀에서 액터를 가져오거나 생성함
	AActor *SpawnFromPool(UClass *ActorClass, FVector Location, FRotator Rotation);

	// 사용이 끝난 액터를 풀로 반환함
	void ReturnToPool(AActor *Actor);

	UFUNCTION(BlueprintCallable, Category = "Pooling")
	void PrewarmPool(UClass *ActorClass, int32 Quantity);

private:
	UPROPERTY()
	TMap<UClass *, FPoolArray> ObjectPools;
};
