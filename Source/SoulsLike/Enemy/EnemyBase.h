// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CombatComponent.h"
#include "SoulsLike/Component/AttributeComponent.h"
#include "SoulsLike/SoulsLikesTypes.h"
#include "Sound/SoundBase.h"
#include "SoulsLike/Enemy/EnemyDataAsset.h"
#include "EnemyBase.generated.h"
DECLARE_MULTICAST_DELEGATE(FOnDie);

struct FTimerHandle;
class UBossHealthBar;
UCLASS()
class SOULSLIKE_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()
public:
	void PlayMeshJitter(float Intensity = 5.0f, float Duration = 0.15f);
	FOnDie OnDie;

	UFUNCTION()
	void OnHealthChangedReceived(float CurrentHealth, float MaxHealth);

	UFUNCTION()
	void OnStaminaChangedReceived(float CurrentStamina, float MaxStamina);

protected:
	void HandleMeshJitter();
	void RestoreMeshPosition();

private:
	FTimerHandle JitterTimerHandle;
	FTimerHandle JitterRestoreTimerHandle;

	FVector OriginalMeshLocation;
	float CurrentJitterIntensity;

public:
	// Sets default values for this pawn's properties
	AEnemyBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UCombatComponent> CombatComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attribute")
	TObjectPtr<UAttributeComponent> AttributeComp;

	bool bCanMove = true;
	UPROPERTY(EditAnywhere)
	bool bIsSpawnable = false;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UBossHealthBar> BossHealthBarClass;

	UPROPERTY()
	TObjectPtr<UBossHealthBar> BossHealthBar;

	void HideHealthBar();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool bIsDead = false;
	bool bIsStunned = false;

	bool bIsAttacking = false; // 공격 중인지
	bool bIsHitting = false;   // 피격 중인지
	bool bCanBeHit = true;	   // 피격 가능한지
	bool bIsSwept = false;	   // 스윕을 받는 중인지

	UFUNCTION()
	virtual void OnNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload &Payload);

	UFUNCTION()
	virtual void OnNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload &Payload);

	UPROPERTY(VisibleAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> StunMontage;

	UPROPERTY(VisibleAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> DieMontage;

	bool PlayAttackMontage();

	UPROPERTY(VisibleAnywhere, Category = "SFX")
	TObjectPtr<USoundBase> HitSFX;

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> MeshMIDs;

	FTimerHandle DissolveTimerHandle;

	float CurrentDissolveValue = 1.1f;

	UPROPERTY(VisibleAnywhere, Category = "VFX")
	float DissolveSpeed = 0.5f;

	void StartDissolveEffect();
	void UpdateDissolveEffect();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Hit(float Damage, float StaminaDamage);
	virtual void Die();
	virtual void Stun();
	virtual bool Attack();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;

protected:
	void InitComponents(); // 컴포넌트 연결
	void SetupBindings();  // 델리게이트 연결
	void InitStats();	   // 데이터 에셋 적용

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	TObjectPtr<UEnemyDataAsset> EnemyData;
};
