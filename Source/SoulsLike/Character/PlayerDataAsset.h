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
	UPROPERTY(EditAnywhere, Category = "Locomotion", meta = (DisplayName = "최대 걷기 속도", UIMin = "0.0", UIMax = "1000.0", ClampMin = "0.0", ClampMax = "2000.0"))
	float MaxWalkSpeed = 450.0f;

	UPROPERTY(EditAnywhere, Category = "Locomotion", meta = (DisplayName = "최대 달리기 속도", UIMin = "0.0", UIMax = "2000.0", ClampMin = "0.0", ClampMax = "4000.0"))
	float MaxSprintSpeed = 1100.0f;

	/** --- [전투 스탯 및 판정] --- */
	UPROPERTY(EditAnywhere, Category = "Combat|Stats", meta = (DisplayName = "가드 데미지 감소율", UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "1.0"))
	float GuardRate = 0.60f;

	UPROPERTY(EditAnywhere, Category = "Combat|Stats", meta = (DisplayName = "일반 공격 속도", UIMin = "0.1", UIMax = "2.5", ClampMin = "0.1", ClampMax = "10.0"))
	float LightAttackSpeed = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Combat|Sweep", meta = (DisplayName = "공격 판정 사거리", UIMin = "0.0", UIMax = "500.0", ClampMin = "0.0", ClampMax = "1000.0"))
	float AttackSweepRange = 120.0f;

	UPROPERTY(EditAnywhere, Category = "Combat|Sweep", meta = (DisplayName = "공격 판정 반경", UIMin = "0.0", UIMax = "50.0", ClampMin = "0.0", ClampMax = "500.0"))
	float AttackSweepRadius = 5.0f;

	/** --- [스테미나 소모 및 회복] --- */
	UPROPERTY(EditAnywhere, Category = "Stamina|Costs", meta = (DisplayName = "약공격 스테미나 소모량", UIMin = "0.0", UIMax = "100.0", ClampMin = "0.0", ClampMax = "500.0"))
	float LightAttackStaminaCost = 6.0f;

	UPROPERTY(EditAnywhere, Category = "Stamina|Costs", meta = (DisplayName = "강공격 스테미나 소모량", UIMin = "0.0", UIMax = "100.0", ClampMin = "0.0", ClampMax = "500.0"))
	float HeavyAttackStaminaCost = 13.0f;

	UPROPERTY(EditAnywhere, Category = "Stamina|Costs", meta = (DisplayName = "회피 스테미나 소모량", UIMin = "0.0", UIMax = "100.0", ClampMin = "0.0", ClampMax = "500.0"))
	float DodgeStaminaCost = 25.0f;

	UPROPERTY(EditAnywhere, Category = "Stamina|Costs", meta = (DisplayName = "패링 스테미나 소모량", UIMin = "0.0", UIMax = "100.0", ClampMin = "0.0", ClampMax = "500.0"))
	float ParryStaminaCost = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Stamina|Costs", meta = (DisplayName = "패링 성공 시 스테미나 회복량", UIMin = "0.0", UIMax = "100.0", ClampMin = "0.0", ClampMax = "500.0"))
	float ParryStaminaReturn = 12.0f;

	UPROPERTY(EditAnywhere, Category = "Stamina|Costs", meta = (DisplayName = "가드 중 스테미나 회복 배율", UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "2.0"))
	float GuardStaminaRegenMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Stamina|Costs", meta = (DisplayName = "가드 시 피해 대비 스테미나 소모율", UIMin = "0.0", UIMax = "5.0", ClampMin = "0.0", ClampMax = "20.0"))
	float GuardStaminaCostRate = 1.75f;

	UPROPERTY(EditAnywhere, Category = "Stamina|Costs", meta = (DisplayName = "달리기 시작 스테미나 소모량", UIMin = "0.0", UIMax = "10.0", ClampMin = "0.0", ClampMax = "200.0"))
	float SprintActivationCost = 2.5f;

	UPROPERTY(EditAnywhere, Category = "Stamina|Costs", meta = (DisplayName = "달리기 유지 스테미나 소모량", UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "10.0"))
	float SprintStaminaCost = 0.08f;

	/** --- [비주얼 이펙트 (VFX)] --- */
	UPROPERTY(EditAnywhere, Category = "VFX", meta = (DisplayName = "약공격 피격 이펙트 (Niagara)"))
	TObjectPtr<UNiagaraSystem> LightAttackImpactVFX;

	UPROPERTY(EditAnywhere, Category = "VFX", meta = (DisplayName = "강공격 피격 이펙트 (Niagara)"))
	TObjectPtr<UNiagaraSystem> HeavyAttackImpactVFX;

	UPROPERTY(EditAnywhere, Category = "VFX", meta = (DisplayName = "카운터 피격 이펙트 (Niagara)"))
	TObjectPtr<UNiagaraSystem> CounterAttackImpactVFX;

	UPROPERTY(EditAnywhere, Category = "VFX", meta = (DisplayName = "기본 피격 파티클 (Cascade)"))
	TObjectPtr<UParticleSystem> ImpactParticle;

	UPROPERTY(EditAnywhere, Category = "VFX", meta = (DisplayName = "가드 피격 파티클"))
	TObjectPtr<UParticleSystem> GuardImpactVFX;

	UPROPERTY(EditAnywhere, Category = "VFX", meta = (DisplayName = "패링 성공 파티클"))
	TObjectPtr<UParticleSystem> ParryImpactVFX;

	/** --- [애니메이션 몽타주] --- */
	UPROPERTY(EditAnywhere, Category = "Animations|Combat", meta = (DisplayName = "일반 공격 콤보 목록"))
	TArray<TObjectPtr<UAnimMontage>> AttackMontages;

	UPROPERTY(EditAnywhere, Category = "Animations|Combat", meta = (DisplayName = "강공격 몽타주"))
	TObjectPtr<UAnimMontage> HeavyAttackMontage;

	UPROPERTY(EditAnywhere, Category = "Animations|Combat", meta = (DisplayName = "대쉬 공격 몽타주"))
	TObjectPtr<UAnimMontage> DashAttackMontage;

	UPROPERTY(EditAnywhere, Category = "Animations|Combat", meta = (DisplayName = "패링 시도 몽타주"))
	TObjectPtr<UAnimMontage> ParryMontage;

	UPROPERTY(EditAnywhere, Category = "Animations|Combat", meta = (DisplayName = "패링 성공 액션 몽타주"))
	TObjectPtr<UAnimMontage> ParryActivationMontage;

	UPROPERTY(EditAnywhere, Category = "Animations|Combat", meta = (DisplayName = "카운터 공격 몽타주"))
	TObjectPtr<UAnimMontage> CounterMontage;

	UPROPERTY(EditAnywhere, Category = "Animations|Combat", meta = (DisplayName = "스턴 몽타주"))
	TObjectPtr<UAnimMontage> StunMontage;

	UPROPERTY(EditAnywhere, Category = "Animations|Combat", meta = (DisplayName = "회피 몽타주"))
	TObjectPtr<UAnimMontage> DodgeMontage;

	UPROPERTY(EditAnywhere, Category = "Animations|Combat", meta = (DisplayName = "방향별 피격 몽타주 맵"))
	TMap<EAttackDirection, TObjectPtr<UAnimMontage>> HitMontages;

	UPROPERTY(EditAnywhere, Category = "Animations|Combat", meta = (DisplayName = "방향별 가드 피격 몽타주 맵"))
	TMap<EAttackDirection, TObjectPtr<UAnimMontage>> HitGuardMontages;
};
