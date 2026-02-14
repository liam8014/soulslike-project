// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SoulsLike/SoulsLikesTypes.h"
#include "NiagaraSystem.h"
#include "PlayerCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class APlayerController;
class USpringArmComponent;
class UCameraComponent;
class UCharacterMovementComponent;
class UStatusBar;
struct FInputActionValue;

class UPlayerAttributeComponent;
class UPlayerCombatComponent;
class UPlayerDataAsset;
template <typename TEnum>
static FText EnumDisplayName(TEnum Value)
{
	if (const UEnum *E = StaticEnum<TEnum>())
		return E->GetDisplayNameTextByValue(static_cast<int64>(Value));
	return FText::GetEmpty();
}

UCLASS()
class SOULSLIKE_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

	UPROPERTY(EditDefaultsOnly)
	UPlayerDataAsset *PlayerDataAsset;

	/* 플레이어 상태 */
	EMovementState MovementState = EMovementState::MS_Idle;
	void SetMovementState(EMovementState NewMovementState);
	bool CheckMovementState(EMovementState _MovementState);
	EMovementState GetMovementState();

	bool CanBeHit();
	void EnableCounterAttack();
	void Die();

	/* UI */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> FocusIndicatorWidgetClass;

	UPROPERTY()
	UUserWidget *FocusIndicatorWidget;

	/* 어트리뷰트 */
	UFUNCTION()
	void OnHealthChangedReceived(float CurrentHealth, float MaxHealth);

	UFUNCTION()
	void OnStaminaChangedReceived(float CurrentStamina, float MaxStamina);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attribute")
	UPlayerAttributeComponent *AttrComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UPlayerCombatComponent *CombatComp;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UStatusBar> StatusBarClass;

	UPROPERTY()
	UStatusBar *StatusBar;

	void HideUI();
	void ShowUI();

	/* 전투 (피격)*/
	EHitResult Hit(const FGameplayHitInfo &HitInfo); // 피격

	/* 카메라 및 포커싱 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	bool bIsFocusing = false; // 플레이어가 포커싱 중인지 여부

	bool bIsCounterTiming = false; // 공격을 받기 직전인지(회피 타이밍)

public: // camera
	UPROPERTY(EditAnywhere, Category = "Camera|Effect")
	FVector DefaultSocketOffset; // 게임 시작 시 자동 저장됨

	UPROPERTY(EditAnywhere, Category = "Camera|Effect")
	FVector CounterReadySocketOffset = FVector(0.f, -50.f, -20.f); // 예: 살짝 오른쪽 아래로

	UPROPERTY(EditAnywhere, Category = "Camera|Effect")
	float CameraInterpSpeed = 5.0f; // 카메라 이동 속도

	float DefaultFOV; // 게임 시작 시 자동 저장됨
	float TargetFOV;  // 현재 목표 FOV

	UPROPERTY(EditAnywhere, Category = "Camera|Effect")
	float CounterActionFOV = 110.0f; // 질주감을 위한 넓은 FOV

	UPROPERTY(EditAnywhere, Category = "Camera|Effect")
	float FOVInterpSpeed = 15.0f; // FOV 변화 속도

	FTimerHandle FOVRestoreTimerHandle; // FOV 복구용 타이머

	void RestoreFOV();
	void ChangeFOV(float FOV);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void InitComponents();
	void SetupBindings();

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
	TObjectPtr<UInputAction> AttackAction;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> HeavyAttackAction;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> RunAction;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> DodgeAction;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> GuardAction;

	void Move(const FInputActionValue &Value);				// 캐릭터의 위치를 이동한다
	void Look(const FInputActionValue &Value);				// 캐릭터의 카메라 방향을 조종한다
	void SwitchFocus(const FInputActionValue &Value);		// 캐릭터의 포커싱 활용 여부를 전환한다
	void ChangeFocusTarget(const FInputActionValue &Value); // 캐릭터의 포커싱 타겟을 바꾼다

	void Attack();		// 상태에 따라 공격을 한다
	void HeavyAttack(); // 강공격을 한다
	void Dodge();		// 캐릭터가 회피한다

	void Jump() override; // 공격 중 점프 비활성화를 위한 오버라이드
	void Run();			  // 캐릭터의 이동속도 상한을 증가시킨다
	void StopRunning();	  // 캐릭터의 이동속도 상한을 기본 속도로 설정한다

	/* 카메라 및 포커싱 */
	void ToogleFocus();						 //  SwitchFocus의 실제 구현부
	bool SearchFocusTarget();				 // 포커싱 타겟을 탐색하여 성공 여부를 반환한다
	void UpdateFocusCamera(float DeltaTime); // 카메라를 포커싱에 맞게 업데이트한다

	TArray<APawn *> FocusTargetArray; // 포커싱 타겟을 저장하는 배열
	int32 CurrentFocusIndex = 0;	  // 현재 포커싱 중인 Pawn의 Index
	UPROPERTY(EditAnywhere)
	float FocusSearchRadius = 3000.f; // 포커싱 범위

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent *CameraBoom; // 스프링 암 컴포넌트

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent *FollowCamera; // 카메라 컴포넌트

	/* 애니메이션 */
	UFUNCTION()
	void OnNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload &Payload);

	UFUNCTION()
	void OnNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload &Payload);

public:
	/* 조작 */
	UCharacterMovementComponent *MoveComp;
	bool CanMove();
	void ResetMovement();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Tuning")
	float SideMovementMultiplier = 0.5f; // 좌/우 성분 계수

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Tuning")
	float BackwardMovementMultiplier = 0.3f; // 뒤로 이동(정면 반대) 성분 계수

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Tuning")
	float AlignmentThreshold = 0.9f; // 전방 기준 임계

	bool bIsRunning = false;

	/* 조작(점프) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bJustLanded = false;

	virtual void Landed(const FHitResult &Hit) override;

	void Guard();
	void StopGuarding();
	/* 더미 기능 */
	UPROPERTY(EditAnywhere, Category = "Dummy")
	bool bAutoAttack = false;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;
};
