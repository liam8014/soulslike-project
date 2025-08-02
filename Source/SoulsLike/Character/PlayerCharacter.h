// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class APlayerController;
class USpringArmComponent;
class UCameraComponent;
struct FInputActionValue;

UCLASS()
class SOULSLIKE_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

	/* UI */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> FocusIndicatorWidgetClass;

	UPROPERTY()
	UUserWidget *FocusIndicatorWidget;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	APlayerController *PlayerController; // 플레이어 컨트롤러

	/* 입력 액션 및 매핑 */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> MovingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> SwitchFocusAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> ChangeFocusTargetAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> LightAttackAction;

	void Move(const FInputActionValue &Value);				// 캐릭터의 위치를 이동한다
	void Look(const FInputActionValue &Value);				// 캐릭터의 카메라 방향을 조종한다
	void SwitchFocus(const FInputActionValue &Value);		// 캐릭터의 포커싱 활용 여부를 전환한다
	void ChangeFocusTarget(const FInputActionValue &Value); // 캐릭터의 포커싱 타겟을 바꾼다
	void LightAttack(const FInputActionValue &Value);		// 기본 공격(약한 공격)을 한다

	/* 카메라 및 포커싱 */
	void ToogleFocus();						 //  SwitchFocus의 실제 구현부
	bool SearchFocusTarget();				 // 포커싱 타겟을 탐색하여 성공 여부를 반환한다
	void UpdateFocusCamera(float DeltaTime); // 카메라를 포커싱에 맞게 업데이트한다

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	bool bIsFocusing = false;		  // 플레이어가 포커싱 중인지 여부
	TArray<APawn *> FocusTargetArray; // 포커싱 타겟을 저장하는 배열
	int32 CurrentFocusIndex = 0;	  // 현재 포커싱 중인 Pawn의 Index
	UPROPERTY(EditAnywhere)
	float FocusSearchRadius = 3000.f; // 포커싱 범위

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent *CameraBoom; // 스프링 암 컴포넌트

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent *FollowCamera; // 카메라 컴포넌트

	/* 전투 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bIsAttacking = false; // 공격 중인지
	bool bCanCombo = false;	   // 콤보를 연속할 수 있는지
	bool bWantCombo = false;   // 플레이어가 콤보 연속을 원하는지

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	int32 CurrentAttackCombo = 0; // 현재 콤보 수

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TArray<UAnimMontage *> AttackMontages; // 콤보 몽타주 레퍼런스

	void PlayAttackMontage();

	UFUNCTION()
	void OnNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload &Payload);

	UFUNCTION()
	void OnNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload &Payload);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;
};
