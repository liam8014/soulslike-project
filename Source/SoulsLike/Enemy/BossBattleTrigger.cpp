#include "BossBattleTrigger.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "EnemyBase.h" // 보스 헤더
#include "SoulsLike/Character/PlayerCharacter.h"
#include "Blueprint/UserWidget.h"

ABossBattleTrigger::ABossBattleTrigger()
{
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;

	// 트리거 설정
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
}

void ABossBattleTrigger::BeginPlay()
{
	Super::BeginPlay();
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ABossBattleTrigger::OnTriggerOverlap);

	// 시작 시 벽은 뚫려있게 설정 (혹은 레벨에서 미리 설정)
	if (BlockingWall)
	{
		BlockingWall->SetActorEnableCollision(false);
		BlockingWall->SetActorHiddenInGame(true);
	}
}

void ABossBattleTrigger::OnTriggerOverlap(UPrimitiveComponent *OverlappedComp, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	if (bIsTriggered)
		return;

	APlayerCharacter *PlayerChar = Cast<APlayerCharacter>(OtherActor);
	if (!PlayerChar)
		return;

	bIsTriggered = true;

	if (BlockingWall)
	{
		BlockingWall->SetActorEnableCollision(true);
		BlockingWall->SetActorHiddenInGame(false);
	}
	PlayerChar->HideUI();

	if (APlayerController *PC = Cast<APlayerController>(PlayerChar->GetController()))
	{
		PC->SetCinematicMode(true, true, true);
	}

	if (BossSequence)
	{
		FMovieSceneSequencePlaybackSettings Settings;
		Settings.bDisableCameraCuts = false;

		ULevelSequencePlayer *SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), BossSequence, Settings, SequenceActor);

		if (SequencePlayer)
		{
			SequencePlayer->OnFinished.AddDynamic(this, &ABossBattleTrigger::OnSequenceFinished);
			SequencePlayer->Play();
		}
	}
	else
	{
		OnSequenceFinished();
	}
}

void ABossBattleTrigger::OnSequenceFinished()
{
	APlayerCharacter *PlayerChar = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (PlayerChar)
	{
		PlayerChar->ShowUI();
		if (APlayerController *PC = Cast<APlayerController>(PlayerChar->GetController()))
		{
			PC->SetCinematicMode(false, true, true);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to find AIC!"));
		}
	}

	if (BossClass && BossSpawnPoint)
	{
		FVector SpawnLoc = BossSpawnPoint->GetActorLocation();
		FRotator SpawnRot = BossSpawnPoint->GetActorRotation();

		SpawnedBoss = GetWorld()->SpawnActor<AEnemyBase>(BossClass, SpawnLoc, SpawnRot);
		if (SpawnedBoss)
		{
			SpawnedBoss->SpawnDefaultController();
		}
	}
}

void ABossBattleTrigger::Tick(float DeltaTime)
{
}
