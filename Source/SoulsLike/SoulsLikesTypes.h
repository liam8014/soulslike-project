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
