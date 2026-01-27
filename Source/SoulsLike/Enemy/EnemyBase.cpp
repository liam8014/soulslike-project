// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyBase.h"
#include "AIController.h"
#include "TimerManager.h"
#include "CombatComponent.h"
void AEnemyBase::PlayMeshJitter(float Intensity, float Duration)
{
	if (!GetMesh())
		return;

	CurrentJitterIntensity = Intensity;

	// 1. 이미 떨리고 있다면 타이머 초기화 (연타 맞았을 때 갱신)
	GetWorld()->GetTimerManager().ClearTimer(JitterTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(JitterRestoreTimerHandle);

	// 2. 아주 빠른 간격(0.01초 ~ 0.02초)으로 위치를 바꾸는 타이머 시작
	GetWorld()->GetTimerManager().SetTimer(
		JitterTimerHandle,
		this,
		&AEnemyBase::HandleMeshJitter,
		0.015f, // 0.015초마다 흔들림 (60fps 기준 매 프레임)
		true	// 반복
	);

	// 3. Duration 뒤에 멈추는 타이머 설정
	GetWorld()->GetTimerManager().SetTimer(
		JitterRestoreTimerHandle,
		this,
		&AEnemyBase::RestoreMeshPosition,
		Duration,
		false);
}
void AEnemyBase::HandleMeshJitter()
{
	if (!GetMesh())
		return;

	// 랜덤 벡터 생성 (-1 ~ 1 사이의 랜덤 값 * 강도)
	FVector RandomOffset = FMath::VRand() * CurrentJitterIntensity;

	// 원래 위치 + 랜덤 오프셋 적용
	GetMesh()->SetRelativeLocation(OriginalMeshLocation + RandomOffset);
}
void AEnemyBase::RestoreMeshPosition()
{
	// 떨림 타이머 종료
	GetWorld()->GetTimerManager().ClearTimer(JitterTimerHandle);

	// 메시를 원래 위치로 깔끔하게 복구
	if (GetMesh())
	{
		GetMesh()->SetRelativeLocation(OriginalMeshLocation);
	}
}
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
	if (GetMesh())
	{
		OriginalMeshLocation = GetMesh()->GetRelativeLocation();
	}
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
	PlayMeshJitter(1, 0.05f);
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
