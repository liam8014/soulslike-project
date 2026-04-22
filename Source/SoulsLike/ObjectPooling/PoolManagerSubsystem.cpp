// Fill out your copyright notice in the Description page of Project Settings.

#include "PoolManagerSubsystem.h"
#include "PoolableInterface.h"

AActor *UPoolManagerSubsystem::SpawnFromPool(UClass *ActorClass, FVector Location, FRotator Rotation)
{
    if (!ActorClass)
        return nullptr;

    FPoolArray &Pool = ObjectPools.FindOrAdd(ActorClass);
    AActor *SpawnedActor = nullptr;

    if (Pool.InactiveActors.Num() > 0)
    {
        SpawnedActor = Pool.InactiveActors.Pop();
        SpawnedActor->SetActorLocationAndRotation(Location, Rotation);
    }
    else
    {
        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        SpawnedActor = GetWorld()->SpawnActor<AActor>(ActorClass, Location, Rotation, Params);
    }

    if (SpawnedActor)
    {
        if (IPoolableInterface *Poolable = Cast<IPoolableInterface>(SpawnedActor))
        {
            Poolable->OnSpawnFromPool();
        }

        // 활성 카운트 증가 및 로그 출력
        Pool.ActiveCount++;
        UE_LOG(LogTemp, Log, TEXT("[%s] Spawned. PoolSize(Inactive): %d, ActiveCount: %d, TotalSize : %d"),
               *ActorClass->GetName(), Pool.InactiveActors.Num(), Pool.ActiveCount, Pool.InactiveActors.Num() + Pool.ActiveCount);
    }

    return SpawnedActor;
}

void UPoolManagerSubsystem::ReturnToPool(AActor *Actor)
{
    if (!Actor)
        return;

    UClass *ActorClass = Actor->GetClass();
    FPoolArray &Pool = ObjectPools.FindOrAdd(ActorClass);

    if (IPoolableInterface *Poolable = Cast<IPoolableInterface>(Actor))
    {
        Poolable->OnReturnToPool();
    }

    Pool.InactiveActors.Add(Actor);

    // 활성 카운트 감소 및 로그 출력
    Pool.ActiveCount = FMath::Max(0, Pool.ActiveCount - 1); // 안전을 위해 0 이하 방지
    UE_LOG(LogTemp, Log, TEXT("[%s] Returned. PoolSize(Inactive): %d, ActiveCount: %d, TotalSize : %d"),
           *ActorClass->GetName(), Pool.InactiveActors.Num(), Pool.ActiveCount, Pool.InactiveActors.Num() + Pool.ActiveCount);
}

void UPoolManagerSubsystem::PrewarmPool(UClass *ActorClass, int32 Quantity)
{
    if (!ActorClass || !GetWorld())
        return;

    FPoolArray &Pool = ObjectPools.FindOrAdd(ActorClass);

    for (int32 i = 0; i < Quantity; i++)
    {
        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        AActor *NewActor = GetWorld()->SpawnActor<AActor>(ActorClass, FVector::ZeroVector, FRotator::ZeroRotator, Params);

        if (NewActor)
        {

            if (IPoolableInterface *Poolable = Cast<IPoolableInterface>(NewActor))
            {
                Poolable->OnReturnToPool();
            }
            Pool.InactiveActors.Add(NewActor);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[%s] Prewarmed %d instances. Inactive Pool Size: %d"),
           *ActorClass->GetName(), Quantity, Pool.InactiveActors.Num());
}
