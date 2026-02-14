// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/GameplayStatics.h"
#include "Components/ActorComponent.h"
#include "SoulsLike/SoulsLikesTypes.h"
#include "PlayerCombatComponent.generated.h"

class APlayerCharacter;
class UParticleSystem;
class UPlayerAttributeComponent;
class UNiagaraSystem;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SOULSLIKE_API UPlayerCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPlayerCombatComponent();

	// --- [외부 제어 함수] ---
	void EnableAttackSweep();
	void DisableAttackSweep();

	void EnableAttack();
	void DisableAttack();

	void EnableAttackCombo();
	void DisableAttackCombo();

	void EnableParry();
	void DisableParry();

	void EnableGuard();
	void DisableGuard();

	UFUNCTION(BlueprintCallable)
	bool IsGuarding() const;

	void EnableCounterAttack();
	void DisableCounterAttack();

	UFUNCTION(BlueprintCallable)
	bool CanCounterAttack() const;

	void EnableDodge();
	void DisableDodge();

	bool CanAttack() const;
	bool CanBeHit() const;
	void RequestAttack(); // 공격 분기

	void LightAttack(); // 기본 공격(약한 공격)
	void HeavyAttack(); // 강공격

	void Dodge();		  // 캐릭터가 회피한다
	void CounterAttack(); // 회피 저스트 발동 시 카운터 공격

	void Parry();
	void Guard();
	void StopGuarding();

	void Stun();

	void ResetMovement();

	EHitResult Hit(const FGameplayHitInfo &HitInfo);
	void PlayHitMontage(EAttackDirection ad);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

protected:
	// --- [내부 멤버 변수] ---
	UPROPERTY()
	APlayerCharacter *Owner;

	UPROPERTY()
	UPlayerAttributeComponent *OwnerAttrComp;

	// 메인 스윕 함수 (Tick에서 호출)
	void AttackTrace();

	// 물리 연산 실행
	bool PerformAttackSweep(TArray<FHitResult> &OutHits);

	// 개별 히트 처리
	void ProcessHit(const FHitResult &HitResult);

	// 동작 플래그
	bool bIsSweeping = false;
	bool bIsDodging = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement") // 애님그래프 연동용
	bool bIsGuarding = false;
	bool bCanBeHit = true;
	bool bIsParrying = false;
	bool bCanCounterAttack = false;
	bool bIsCounterTiming = false;
	bool bCanAttack = true;
	bool bCanCombo = false;
	bool bWantCombo = false;

	// 중복 피격 방지
	TSet<AActor *> ProcessedActors;

	// 런타임 사용 변수
	float DamageMultiplier = 1.0f;
	float StaminaMultiplier = 1.0f;
	EAttackDirection AttackDirection;
	UNiagaraSystem *AttackImpactVFX;
	int32 CurrentAttackCombo = 0; // 현재 콤보 수

	void SetAttackAttribute(float DMultiplier, float SMultiplier, EAttackDirection Direction, UNiagaraSystem *Niagara);

	UPROPERTY(EditAnywhere, Category = "VFX")
	UNiagaraSystem *LightAttackImpactVFX;

	UPROPERTY(EditAnywhere, Category = "VFX")
	UNiagaraSystem *HeavyAttackImpactVFX;

	UPROPERTY(EditAnywhere, Category = "VFX")
	UNiagaraSystem *CounterAttackImpactVFX;

	UPROPERTY(EditAnywhere, Category = "VFX")
	UParticleSystem *ImpactParticle;

	UPROPERTY(EditAnywhere, Category = "VFX")
	UParticleSystem *GuardImpactVFX;

	UPROPERTY(EditAnywhere, Category = "VFX")
	UParticleSystem *ParryImpactVFX;

	UStaticMeshComponent *SwordMeshComponent; // 검의 메쉬 컴포넌트

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawDebugShape = true;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<UAnimMontage *> AttackMontages; // 콤보 몽타주

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage *HeavyAttackMontage; // 강공격 몽타주

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage *DashAttackMontage; // 대쉬공격 몽타주

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage *ParryMontage; // 패링 몽타주

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage *ParryActivationMontage; // 패링 발동 몽타주

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage *ReadyCounterMontage; // 카운터 준비 몽타주

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage *ReleaseCounterMontage; // 카운터 준비 해제 몽타주

	UPROPERTY(EditAnywhere, Category = "Combat")
	TMap<EAttackDirection, UAnimMontage *> HitMontages; // 피격 몽타주

	UPROPERTY(EditAnywhere, Category = "Combat")
	TMap<EAttackDirection, UAnimMontage *> HitGuardMontages; // 피격(가드) 몽타주

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage *BlockedMontage; // 공격 막힘 몽타주

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage *CounterMontage; // 카운터 공격 몽타주

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage *StunMontage; // 스턴 몽타주

	UPROPERTY(EditAnywhere, Category = "Movement")
	UAnimMontage *DodgeMontage; // 회피 몽타주

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
};
