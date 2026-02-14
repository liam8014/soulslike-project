#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SoulsLike/SoulsLikesTypes.h" // EAttackDirection 등 사용
#include "PlayerDataAsset.generated.h"

class UAnimMontage;
class UNiagaraSystem;
class UParticleSystem;

UCLASS()
class SOULSLIKE_API UPlayerDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** --- [이동 및 로코모션] --- */
	UPROPERTY(EditAnywhere, Category = "Locomotion")
	float MaxWalkSpeed = 450.0f;

	UPROPERTY(EditAnywhere, Category = "Locomotion")
	float MaxSprintSpeed = 1100.0f;

	/** --- [전투 스탯 및 판정] --- */
	UPROPERTY(EditAnywhere, Category = "Combat|Stats")
	float GuardRate = 0.60f; // 가드 데미지 감소율

	UPROPERTY(EditAnywhere, Category = "Combat|Stats")
	float LightAttackSpeed = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Combat|Sweep")
	float AttackSweepRange = 120.0f;

	UPROPERTY(EditAnywhere, Category = "Combat|Sweep")
	float AttackSweepRadius = 5.0f;

	/** --- [스테미나 소모 및 회복] --- */
	UPROPERTY(EditAnywhere, Category = "Stamina|Costs")
	float LightAttackStaminaCost = 6.0f;

	UPROPERTY(EditAnywhere, Category = "Stamina|Costs")
	float HeavyAttackStaminaCost = 13.0f;

	UPROPERTY(EditAnywhere, Category = "Stamina|Costs")
	float DodgeStaminaCost = 25.0f;

	UPROPERTY(EditAnywhere, Category = "Stamina|Costs")
	float ParryStaminaCost = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Stamina|Regen")
	float ParryStaminaReturn = 12.0f;

	UPROPERTY(EditAnywhere, Category = "Stamina|Regen")
	float GuardStaminaRegenMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Stamina|Costs")
	float GuardStaminaCostRate = 1.75f; // 피해량 대비 스테미나 소모율

	UPROPERTY(EditAnywhere, Category = "Stamina|Costs")
	float SprintActivationCost = 2.5f;

	UPROPERTY(EditAnywhere, Category = "Stamina|Costs")
	float SprintStaminaCost = 0.08f;

	/** --- [비주얼 이펙트 (VFX)] --- */
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

	/** --- [애니메이션 몽타주] --- */
	UPROPERTY(EditAnywhere, Category = "Animations|Attack")
	TArray<UAnimMontage *> AttackMontages;

	UPROPERTY(EditAnywhere, Category = "Animations|Attack")
	UAnimMontage *HeavyAttackMontage;

	UPROPERTY(EditAnywhere, Category = "Animations|Attack")
	UAnimMontage *DashAttackMontage;

	UPROPERTY(EditAnywhere, Category = "Animations|Combat")
	UAnimMontage *ParryMontage;

	UPROPERTY(EditAnywhere, Category = "Animations|Combat")
	UAnimMontage *ParryActivationMontage;

	UPROPERTY(EditAnywhere, Category = "Animations|Combat")
	UAnimMontage *CounterMontage;

	UPROPERTY(EditAnywhere, Category = "Animations|Combat")
	UAnimMontage *StunMontage;

	UPROPERTY(EditAnywhere, Category = "Animations|Movement")
	UAnimMontage *DodgeMontage;

	UPROPERTY(EditAnywhere, Category = "Animations|Hit")
	TMap<EAttackDirection, UAnimMontage *> HitMontages;

	UPROPERTY(EditAnywhere, Category = "Animations|Hit")
	TMap<EAttackDirection, UAnimMontage *> HitGuardMontages;
};