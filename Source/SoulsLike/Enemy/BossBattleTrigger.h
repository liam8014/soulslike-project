// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelSequence.h"
#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"
#include "Sound/SoundBase.h"
#include "BossBattleTrigger.generated.h"

UCLASS()
class SOULSLIKE_API ABossBattleTrigger : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABossBattleTrigger();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// 1. 트리거 영역 (플레이어가 밟으면 시작)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UBoxComponent *TriggerBox;

	// 2. 도망 못 가게 막는 벽 (레벨에 배치된 액터를 에디터에서 지정)
	UPROPERTY(EditInstanceOnly, Category = "BossEvent")
	AActor *BlockingWall;

	// 3. 스폰할 보스 클래스
	UPROPERTY(EditAnywhere, Category = "BossEvent")
	TSubclassOf<class AEnemyBase> BossClass;

	// 4. 보스가 스폰될 위치 (Target Point 등을 지정하거나 좌표 입력)
	UPROPERTY(EditInstanceOnly, Category = "BossEvent")
	AActor *BossSpawnPoint;

	// 5. 실행할 시퀀스 (카메라 연출용)
	UPROPERTY(EditAnywhere, Category = "BossEvent")
	ULevelSequence *BossSequence;

	UFUNCTION()
	void OnTriggerOverlap(UPrimitiveComponent *OverlappedComp, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

	// 시퀀스가 끝났을 때 호출될 함수
	UFUNCTION()
	void OnSequenceFinished();

	UFUNCTION()
	void OnDieReceived();

	UPROPERTY(EditAnywhere, Category = "BossEvent")
	USoundBase *BossBattleBGM;

private:
	UPROPERTY()
	ALevelSequenceActor *SequenceActor; // 재생 중인 시퀀스 액터

	UPROPERTY()
	class AEnemyBase *SpawnedBoss; // 스폰된 보스 저장

	bool bIsTriggered = false; // 중복 실행 방지

	UPROPERTY()
	UAudioComponent *BGMComponent;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
