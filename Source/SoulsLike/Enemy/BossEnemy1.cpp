// Fill out your copyright notice in the Description page of Project Settings.

#include "BossEnemy1.h"
#include "GameFramework/CharacterMovementComponent.h"

ABossEnemy1::ABossEnemy1()
{
}

void ABossEnemy1::BeginPlay()
{
    Super::BeginPlay();
    if (UAnimInstance *AnimInst = GetMesh()->GetAnimInstance())
    {
        AnimInst->OnPlayMontageNotifyBegin.AddDynamic(this, &ABossEnemy1::OnNotifyBegin); // NotifyState 시작(NotifyBegin) 바인딩
        AnimInst->OnPlayMontageNotifyEnd.AddDynamic(this, &ABossEnemy1::OnNotifyEnd);     // NotifyState 종료(NotifyEnd) 바인딩
        UE_LOG(LogTemp, Display, TEXT("AnimInst is Set!"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("No AnimInst!"));
    }
}

bool ABossEnemy1::Attack(int32 AttackIndex)
{
    if (!Super::Attack(AttackIndex))
    {
        return false;
    }

    switch (AttackIndex)
    {
    case 0:
        PlayAnimMontage(AttackMontages[0]);
        bIsAttacking = true;
        UE_LOG(LogTemp, Display, TEXT("Attack! %d"), AttackIndex);
        break;
    }

    return true;
}

void ABossEnemy1::OnNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload &Payload)
{
    if (NotifyName == TEXT("Next"))
    {
        UE_LOG(LogTemp, Display, TEXT("Next!"));
        PlayAnimMontage(AttackMontages[1]);
    }
}

void ABossEnemy1::OnNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload &Payload)
{
}