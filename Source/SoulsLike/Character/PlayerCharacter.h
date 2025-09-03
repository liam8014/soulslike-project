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
class UCharacterMovementComponent;
struct FInputActionValue;

UENUM(BlueprintType)
enum class EMovementState : uint8
{
	MS_Idle UMETA(DisplayName = "Idle"),		   // 대기 상태
	MS_Moving UMETA(DisplayName = "Moving"),	   // 이동 상태
	MS_Attacking UMETA(DisplayName = "Attacking"), // 공격 상태
	MS_Hit UMETA(DisplayName = "Hit"),			   // 피격 상태
	MS_Dodging UMETA(DisplayName = "Dodging"),	   // 회피 상태
	MS_Jumping UMETA(DisplayName = "Jumping"),	   // 점프 초기 상태
	MS_Falling UMETA(DisplayName = "Falling"),	   // 낙하 상태
	MS_Guarding UMETA(DisplayName = "Guarding"),   // 가드 상태
	MS_Dead UMETA(DisplayName = "Dead"),		   // 사망 상태
};

UENUM(BlueprintType)
enum class EAttackDirection : uint8
{
	AD_Left UMETA(DisplayName = "Idle"),	// 좌측
	AD_Right UMETA(DisplayName = "Idle"),	// 우측
	AD_Forward UMETA(DisplayName = "Idle"), // 정면
};

UCLASS()
class SOULSLIKE_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

	/* 플레이어 상태 */
	EMovementState MovementState = EMovementState::MS_Idle;

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
	void LightAttack(const FInputActionValue &Value);		// 기본 공격(약한 공격)을 한다
	void Jump() override;									// 공격 중 점프 비활성화를 위한 오버라이드
	void Run();												// 캐릭터의 이동속도 상한을 증가시킨다
	void StopRunning();										// 캐릭터의 이동속도 상한을 기본 속도로 설정한다
	void Dodge();											// 캐릭터가 회피한다

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

	/* 애니메이션 */
	UFUNCTION()
	void OnNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload &Payload);

	UFUNCTION()
	void OnNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload &Payload);

	/* 체력 및 스테미나 */
	float Health = 100.0f;
	float Stamina = 100.0f;

	/* 전투 (공격)*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bIsAttacking = false; // 공격 중인지
	bool bCanAttack = true;	   // 공격 할 수 있는지(동작 쿨타임 확인용)
	bool bCanCombo = false;	   // 콤보를 연속할 수 있는지
	bool bWantCombo = false;   // 플레이어가 콤보 연속을 원하는지
	bool bIsHit = false;	   // 플레이어가 공격을 맞췄는지

	int32 CurrentAttackCombo = 0; // 현재 콤보 수

	UPROPERTY(EditAnywhere, Category = "Combat")
	float LightAttackRange = 120.0f; // 기본 공격 범위

	UStaticMeshComponent *SwordMeshComponent; // 검의 메쉬 컴포넌트

	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<UAnimMontage *> AttackMontages; // 콤보 몽타주 레퍼런스

	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<UAnimMontage *> HitMontages; // 피격 몽타주 레퍼런스

	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<UAnimMontage *> HitGuardMontages; // 피격(가드) 몽타주 레퍼런스

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage *RunAttackMontage; // 대쉬공격 몽타주 레퍼런스

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage *BlockAttackMontage; // 공격 막힘 몽타주 레퍼런스

	void PlayLightAttackMontage(); // 일반 공격 몽타주 재생
	void DoAttackTrace();		   // 공격 트레이스

	/* 전투 (피격)*/
	TArray<APlayerCharacter *> HitCharacters; // 피격 캐릭터들의 배열

	void DoHitReaction(EAttackDirection ad); // 피격 반응
	// void PlayHitMontage();			  // 피격 몽타주 재생
	void PlayHitMontage(int32 index); // 피격 몽타주 선택 재생
	void ResetHitCharacters();		  // 피격 캐릭터 배열 초기화

	/* 조작 */
	UCharacterMovementComponent *MoveComp;
	bool CanMove();

	/* 조작 (걷기)*/
	float WalkSpeed = 500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Tuning")
	float SideMovementMultiplier = 0.6f; // 좌/우 성분 계수

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Tuning")
	float BackwardMovementMultiplier = 0.5f; // 뒤로 이동(정면 반대) 성분 계수

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Tuning")
	float AlignmentThreshold = 0.9f; // 전방 기준 임계

	/* 조작 (달리기)*/
	float RunSpeed = 1300;
	bool bIsRunning = false;

	/* 조작(점프) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bJustLanded = false;

	virtual void Landed(const FHitResult &Hit) override;

	/* 조작 (회피)*/
	bool bIsDodging = false;
	float DodgeCooldown = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	UAnimMontage *DodgeMontage; // 회피 몽타주 레퍼런스

	/* 조작 (가드/패링)*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsGuarding = false;

	void Guard();
	void StopGuarding();

public:
	// Called every frame
	virtual void
	Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;
};
