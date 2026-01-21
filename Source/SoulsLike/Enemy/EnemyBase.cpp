// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyBase.h"
#include "AIController.h"
#include "CombatComponent.h"
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
	CombatComp = FindComponentByClass<UCombatComponent>();
	if (CombatComp)
	{
		UE_LOG(LogTemp, Display, TEXT("Combat Component is successfully set"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Combat Component set failed!"));
	}

	AttributeComp = FindComponentByClass<UAttributeComponent>();
	if (AttributeComp)
	{
		UE_LOG(LogTemp, Display, TEXT("Attribute Component is successfully set"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Attribute Component set failed!"));
	}

	if (UAnimInstance *AnimInst = GetMesh()->GetAnimInstance())
	{
		AnimInst->OnPlayMontageNotifyBegin.AddDynamic(this, &AEnemyBase::OnNotifyBegin); // NotifyState 시작(NotifyBegin) 바인딩
		AnimInst->OnPlayMontageNotifyEnd.AddDynamic(this, &AEnemyBase::OnNotifyEnd);	 // NotifyState 종료(NotifyEnd) 바인딩
		UE_LOG(LogTemp, Display, TEXT("AnimInst is Set"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No AnimInst!"));
	}
}

void AEnemyBase::OnNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload &Payload)
{
	if (NotifyName == TEXT("Sweep"))
	{
		CombatComp->AttackBeforeTrace();
		CombatComp->EnableAttackSweep();
		UE_LOG(LogTemp, Display, TEXT("Start Sweep"));
	}
	if (NotifyName == TEXT("Dash") || NotifyName == TEXT("Approach"))
	{
		AAIController *AIC = Cast<AAIController>(GetController());
		if (AIC)
		{
			FVector TargetLocation = AIC->GetFocalPoint();
			FVector MyLocation = GetActorLocation();

			FVector Direction = (TargetLocation - MyLocation);
			Direction.Z = 0.0f;
			Direction.Normalize();

			float DashSpeed = NotifyName == TEXT("Dash") ? 3500.0f : 1500.0f;
			LaunchCharacter(Direction * DashSpeed, true, false);

			FRotator LookAtRot = Direction.Rotation();
			SetActorRotation(LookAtRot);
		}
	}
	if (NotifyName == TEXT("EndStun"))
	{
		AttributeComp->EnableChangeStamina();
		bCanMove = true;
	}
	if (NotifyName == TEXT("EnableMove"))
	{
		bCanMove = true;
	}
}

void AEnemyBase::OnNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload &Payload)
{
	if (NotifyName == TEXT("Sweep"))
	{
		CombatComp->DisableAttackSweep();
	}
}

bool AEnemyBase::PlayAttackMontage()
{
	UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayAttackMontage] Anim Instance Error!"));
		return false;
	}
	if (CombatComp->NextAnimMontage == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayAttackMontage] Invalid Attack Type!"));
		return false;
	}
	PlayAnimMontage(CombatComp->NextAnimMontage);
	return true;
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

void AEnemyBase::Hit(float Damage, float StaminaDamage)
{
	AttributeComp->ChangeHealth(-Damage);
	AttributeComp->ChangeStamina(-StaminaDamage);
}
void AEnemyBase::Die()
{
}
void AEnemyBase::Stun()
{
	if (StunMontage)
	{
		PlayAnimMontage(StunMontage, 1.0);
	}
	bCanMove = false;
	AttributeComp->DisableChangeStaimna();
}

bool AEnemyBase::Attack()
{
	if (!CombatComp)
	{
		UE_LOG(LogTemp, Error, TEXT("CombatComp Not Found"));
		return false;
	}
	if (!PlayAttackMontage())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed To Play Attack Montage"));
		return false;
	}
	CombatComp->SetRandomAttackType();
	return true;
}
