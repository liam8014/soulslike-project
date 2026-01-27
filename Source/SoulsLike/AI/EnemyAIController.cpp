// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyAIController.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"
void AEnemyAIController::BeginPlay()
{
    Super::BeginPlay();
}

void AEnemyAIController::OnPossess(APawn *InPawn)
{
    Super::OnPossess(InPawn);
    if (AIBehavior)
    {
        RunBehaviorTree(AIBehavior);
        GetBlackboardComponent()->SetValueAsVector(TEXT("StartLocation"), InPawn->GetActorLocation());
    }
}

void AEnemyAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
