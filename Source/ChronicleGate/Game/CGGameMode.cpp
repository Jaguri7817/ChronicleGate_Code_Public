// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/CGGameMode.h"
#include "CGStageStateBase.h"
#include "CGStageStreamerSystem.h"

ACGGameMode::ACGGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> DefaultPawnClassRef(TEXT("/Script/ChronicleGate.CGCharacterPlayer"));
	if (DefaultPawnClassRef.Class)
	{
		DefaultPawnClass = DefaultPawnClassRef.Class;
	}

	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerClassRef(TEXT("/Script/ChronicleGate.CGCharacterPlayerController"));
	if (PlayerControllerClassRef.Class)
	{
		PlayerControllerClass = PlayerControllerClassRef.Class;
	}

	GameStateClass = ACGStageStateBase::StaticClass();
	bUseSeamlessTravel = true;	// PlayerState 등 자동 보존


}

void ACGGameMode::TravelToDungeon()
{
	if (!HasAuthority()) return;
	if (bIsTraveling) return;

	bIsTraveling = true;

	UWorld* World = GetWorld();
	if (!World) return;

	const FString URL = TEXT("/Game/Maps/Stage");
	World->ServerTravel(URL, /*bAbsolute*/ true);
	UE_LOG(LogTemp, Log, TEXT("[Travel] ServerTravel -> %s"), *URL);
	UE_LOG(LogTemp, Warning, TEXT("[Traveler] Travel To Dungeon"));
}

void ACGGameMode::TravelToLobby()
{
	if (!HasAuthority()) return;
	if (bIsTraveling) return;

	bIsTraveling = true;

	UWorld* World = GetWorld();
	if (!World) return;

	const FString URL = TEXT("/Game/Maps/Demonstration1");
	World->ServerTravel(URL, /*bAbsolute*/ true);
}

void ACGGameMode::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();

	// 던전 트래블 시 다음 스테이지를 미리 로딩
	// 퍼시스턴트 레벨에 한해서 실행
	if (GetWorld()->GetNetMode() != NM_DedicatedServer) return;

	const FString LevelName = GetWorld()->GetMapName();
	UE_LOG(LogTemp, Log, TEXT("[GameMode] Current Map Name : %s"), *LevelName);

	if (PersistentLevelName != LevelName) return;
	
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::StageLoadAfterTravel);
}

void ACGGameMode::StageLoadAfterTravel()
{
	if (GetWorld()->GetNetMode() != NM_DedicatedServer) return;

	UCGStageStreamerSystem* StageStreamer = GetWorld()->GetSubsystem<UCGStageStreamerSystem>();
	if (!StageStreamer) { UE_LOG(LogTemp, Log, TEXT("[GameMode] StageStreamer is Null")); }

	UE_LOG(LogTemp, Log, TEXT("[GameMode] StageLoadAfterTravel Execution and DoStreamToStage"));
	StageStreamer->DoStreamToStage(FirstStage);	// 항상 퍼시스턴트 레벨 -> 맨처음 서브레벨은 'Stage1'로 로드
}
