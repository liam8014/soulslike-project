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
	if (NotifyName == TEXT("Track"))
	{
		AAIController *AIC = Cast<AAIController>(GetController());
		if (AIC)
		{
			// 1. 목표 지점(Focal Point) 가져오기
			FVector TargetLocation = AIC->GetFocalPoint();
			FVector MyLocation = GetActorLocation();

			// 2. 방향 벡터 계산 (Z축 높낮이 차이는 무시하고 수평으로만 돌진)
			FVector Direction = (TargetLocation - MyLocation);
			Direction.Z = 0.0f;	   // 바닥으로 꺼지거나 하늘로 솟지 않게
			Direction.Normalize(); // 길이를 1로 만듦 (방향만 남김)

			// 3. 돌진 속도 설정 (원하는 만큼 빠르게 조절하세요)
			float DashSpeed = 3500.0f;

			// 4. 캐릭터 날리기 (Launch)
			// 첫 번째 true: 기존 XY 속도를 무시하고 덮어씌움 (즉시 방향 전환)
			// 두 번째 false: Z축(점프) 속도는 유지 (false)하거나 덮어씌움 (true)
			LaunchCharacter(Direction * DashSpeed, true, false);

			// [선택 사항] 회전도 즉시 타겟을 보게 맞출까요?
			FRotator LookAtRot = Direction.Rotation();
			SetActorRotation(LookAtRot);
		}
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
