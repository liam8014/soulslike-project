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
	void InitializeFromDataAsset(class UPlayerDataAsset *DataAsset);

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

private:
	UPROPERTY()
	class UPlayerDataAsset *PlayerData;

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
	UPROPERTY(EditAnywhere, Category = "Movement") // 애님그래프 연동용
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

	UStaticMeshComponent *SwordMeshComponent; // 검의 메쉬 컴포넌트

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawDebugShape = true;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
};
