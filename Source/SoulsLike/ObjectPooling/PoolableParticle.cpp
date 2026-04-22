// Fill out your copyright notice in the Description page of Project Settings.

#include "PoolableParticle.h"
#include "Particles/ParticleSystemComponent.h"
#include "SoulsLike/ObjectPooling/PoolManagerSubsystem.h"
// Sets default values
APoolableParticle::APoolableParticle()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	ParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleComponent"));
	RootComponent = ParticleComponent;

	ParticleComponent->OnSystemFinished.AddDynamic(this, &APoolableParticle::OnParticleFinished);
}
void APoolableParticle::OnSpawnFromPool()
{
	SetActorHiddenInGame(false);
	TRACE_BOOKMARK(TEXT("Particle_Spawn_Pool"));
	ParticleComponent->Activate(true);
}

void APoolableParticle::OnReturnToPool()
{
	SetActorHiddenInGame(true);
	TRACE_BOOKMARK(TEXT("Particle_Return_Pool"));
	ParticleComponent->Deactivate();
}

// Called when the game starts or when spawned
void APoolableParticle::BeginPlay()
{
	Super::BeginPlay();
}

void APoolableParticle::OnParticleFinished(UParticleSystemComponent *PSystem)
{
	if (UPoolManagerSubsystem *PoolMgr = GetWorld()->GetSubsystem<UPoolManagerSubsystem>())
	{
		PoolMgr->ReturnToPool(this);
	}
}

// Called every frame
void APoolableParticle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
