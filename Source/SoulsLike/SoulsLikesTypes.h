// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SoulsLikesTypes.generated.h"
UENUM(BlueprintType)
enum class EMovementState : uint8
{
	MS_Idle UMETA(DisplayName = "Idle"),		   // 대기 상태
	MS_Moving UMETA(DisplayName = "Moving"),	   // 이동 상태
	MS_Attacking UMETA(DisplayName = "Attacking"), // 공격 상태
	MS_Hit UMETA(DisplayName = "Hit"),			   // 피격 상태
	MS_Stun UMETA(DisplayName = "Stun"),		   // 스턴 상태
	MS_Dodging UMETA(DisplayName = "Dodging"),	   // 회피 상태
	MS_Jumping UMETA(DisplayName = "Jumping"),	   // 점프 초기 상태
	MS_Falling UMETA(DisplayName = "Falling"),	   // 낙하 상태
	MS_Dead UMETA(DisplayName = "Dead"),		   // 사망 상태
};

UENUM(BlueprintType)
enum class EAttackDirection : uint8
{
	AD_Left UMETA(DisplayName = "Left"),		 // 좌측
	AD_Right UMETA(DisplayName = "Right"),		 // 우측
	AD_Forward UMETA(DisplayName = "Forward"),	 // 정면
	AD_Backward UMETA(DisplayName = "Backward"), // 후면
};

UENUM(BlueprintType)
enum class EHitResult : uint8
{
	HR_CleanHit UMETA(DisplayName = "Clean Hit"), // 정타
	HR_Guard UMETA(DisplayName = "Guarded"),	  // 가드 됨
	HR_Parry UMETA(DisplayName = "Parried"),	  // 패리 됨
};

USTRUCT(BlueprintType)
struct FGameplayHitInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	float DamageAmount; // 데미지

	UPROPERTY(BlueprintReadWrite)
	float KnockBackDistance; // 넉백 거리

	UPROPERTY(BlueprintReadWrite)
	EAttackDirection AttackDirection = EAttackDirection::AD_Forward; // 공격 방향

	UPROPERTY(BlueprintReadWrite)
	FVector HitLocation = FVector::ZeroVector; // 타격 지점 (VFX)

	UPROPERTY(BlueprintReadWrite)
	FVector ImpactNormal = FVector::UpVector; // 타격 지점의 법선 벡터 (VFX 회전값 계산용)

	UPROPERTY(BlueprintReadWrite)
	AActor *DamageCauser = nullptr; // 공격자

	UPROPERTY(BlueprintReadWrite)
	bool bCanBlock = true; // 막을 수 있는 공격인지
};

USTRUCT(BlueprintType)
struct FAttackTypeAttribute
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float MinimumDistance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float WaitTime = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	UAnimMontage *AttackAnim = nullptr;
};
class SOULSLIKE_API SoulsLikesTypes
{

public:
	SoulsLikesTypes();
	~SoulsLikesTypes();
};
