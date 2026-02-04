// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulsLike/Character/PlayerCharacter.h"
#include "SoulsLike/SoulsLikesTypes.h"
#include "Sound/SoundBase.h"
#include "CombatComponent.generated.h"

class UParticleSystem;
class AEnemyBase;
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SOULSLIKE_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();

	// 매 프레임 실행 (스윕 체크)
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	// --- [외부 제어 함수] ---
	void EnableAttackSweep();
	void DisableAttackSweep();
	void AttackBeforeTrace(); // 공격 전조 범위 체크

	// 공격 속성 설정 (애님 노티파이에서 호출)
	void SetAttackAttribute(float Multiplier, float KnockBack, EAttackDirection Direction, FName Socket);

	// 다음 공격 패턴 랜덤 설정
	void SetRandomAttackType();

	// --- [Data Asset 변수] ---
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat | Stats")
	float AttackPower = 10.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat | Stats")
	float AttackRadius = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat | Stats")
	float AttackRange = 70.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat | Stats")
	TArray<FAttackTypeAttribute> AttackPatterns;

	// --- [런타임 상태 변수] ---
	int32 NextAttackType = 0;
	float NextAcceptance = 170.0f;
	float NextWaitTime = 5.0f;
	FName TraceSocket = FName("weapon");

	UPROPERTY()
	UAnimMontage *NextAnimMontage = nullptr;

protected:
	virtual void BeginPlay() override;

private:
	// 메인 스윕 함수 (Tick에서 호출)
	void AttackTrace();

	// 물리 연산 실행
	bool PerformAttackSweep(TArray<FHitResult> &OutHits);

	// 개별 히트 처리
	void ProcessHit(const FHitResult &HitResult);

	// 이펙트 재생
	void SpawnImpactVFX(const FHitResult &Hit, EHitResult HitType);

	// --- [내부 멤버 변수] ---
	UPROPERTY()
	AEnemyBase *Owner;

	// 중복 피격 방지
	TSet<AActor *> ProcessedActors;

	bool bIsSweeping = false;
	bool bIsAttacking = false;

	// 런타임 사용 변수
	float DamageMultiplier = 1.0f;
	float KnockBackDistance = 600.0f;
	EAttackDirection AttackDirection = EAttackDirection::AD_Forward;

	// --- [디버그 설정] ---
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawDebugShape = true;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bShouldFixAttackType = false;

	UPROPERTY(EditAnywhere, Category = "Debug")
	int32 FixedAttackType = 0;

	// --- [VFX 에셋] ---
	UPROPERTY(EditAnywhere, Category = "VFX")
	UParticleSystem *HitImpactVFX;
};
