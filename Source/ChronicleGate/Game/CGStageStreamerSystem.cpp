// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/CGStageStreamerSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"
#include "Engine/LevelStreaming.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "InputCoreTypes.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "CGStageStateBase.h"
#include "Engine/Level.h"               // 추가
#include "CGMonsterSpawner.h"

static ULevelStreaming* FindLevelStreamingByAny(UWorld* World, const FName& PackageName)
{	
	if (!World) return nullptr;
	const FString WantPkg = PackageName.ToString(); // "/Game/Maps/Stage1"
	const FString WantShort = FPackageName::GetShortName(WantPkg); // "Stage1"

	for (ULevelStreaming* LevelStreaming : World->GetStreamingLevels())
	{
		if (!LevelStreaming) continue;

		const FString HavePkg = LevelStreaming->GetWorldAssetPackageName(); // "/Game/Maps/UEDPIE_0_Stage1"
		const FName HaveName = LevelStreaming->GetWorldAssetPackageFName();

		// 1. 전체 경로 일치 확인 (PIE가 아닌 경우 대비)
		if (HaveName == PackageName) return LevelStreaming;

		// 2. PIE 정규화 일치 확인
		FString CleanHavePkg = HavePkg.Replace(TEXT("UEDPIE_0_"), TEXT(""), ESearchCase::IgnoreCase);

		// PIE 접두사 제거 후, 원하는 경로와 일치하는지 확인
		if (CleanHavePkg.Equals(WantPkg, ESearchCase::IgnoreCase))
			return LevelStreaming;
	}
	return nullptr;
	
}

UCGStageStreamerSystem::UCGStageStreamerSystem()
{
}

// ================================================================================================================
// 랜덤 스트리밍 구현부
// ================================================================================================================

ACGStageStateBase* UCGStageStreamerSystem::GetStageState() const
{
	if (!GetWorld()) return nullptr;
	return GetWorld()->GetGameState<ACGStageStateBase>();
}

void UCGStageStreamerSystem::Server_RequestStageStream(const FName& InCurrentPathInState)
{
	// 1. 서버 권한 확인 (UWorld를 통해)
	if (!GetWorld() || GetWorld()->GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Subsystem] Request received on client. Ignoring."));
		return;
	}

	// 2. 스트리밍 중복 실행 방지
	if (bIsStreaming)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Subsystem] Already streaming. Aborting new request."));
		return;
	}

	// 3. 상태 객체 유효성 및 클리어 상태 확인 (GameStateBase 사용)
	ACGStageStateBase* StageState = GetStageState();
	if (!StageState || !StageState->IsStageCleared())
	{
		UE_LOG(LogTemp, Error, TEXT("[Subsystem] StageState invalid or not cleared. Aborting stream request."));
		return;
	}

	// 4. 다음 스테이지 결정 및 유효성 검사
	const FName NextStage = ChooseNextStage(InCurrentPathInState);
	if (NextStage.IsNone())
	{
		UE_LOG(LogTemp, Error, TEXT("[Subsystem] Failed to choose next valid stage. Aborting."));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Subsystem] Choosing Next Stage -> %s"), *NextStage.ToString());

	// 5. 스트리밍 상태 변수 저장 (PrevStagePath : For Unload)
	PrevStagePath = InCurrentPathInState; // 이전 경로를 언로드하기 위한 StageState에 기록된 경로를 이전 경로로 설정

	// 6. 실제 스트리밍 시작 로직 호출
	DoStreamToStage(NextStage);
}

FName UCGStageStreamerSystem::ChooseNextStage(const FName& InCurrentPathInState) const
{
	if (StageVariantMax < StageVariantMin) return NAME_None;

	// 보스 스테이지 -------------------------------------------
	ACGStageStateBase* StageState = GetWorld()->GetGameState<ACGStageStateBase>();
	if (StageState)
	{
		const int32 CurrentStageNum = StageState->GetStageNum();  // 지금까지 클리어/진행한 스테이지 수
		const int32 NextStageNum = CurrentStageNum + 1;
		const int32 MaxStageNum = StageState->GetMaxStageNum();   // 말한 보스 스테이지 번호

		const bool bNextIsBossStage = (NextStageNum == MaxStageNum);
		if (bNextIsBossStage)
		{
			UE_LOG(LogTemp, Warning, TEXT("[ChooseNextStage] Boss Stage -> %s"), *BossStagePath.ToString());
			return BossStagePath;    // /Game/Maps/BossStage1
		}
	}

	// 일반 스테이지 -------------------------------------------
	UE_LOG(LogTemp, Warning, TEXT("[ChooseNextStage] CurrentStagePath: %s"), *InCurrentPathInState.ToString());
	// Stage1 ~ StageN 경로 생성: "/Game/Maps/Stage" + "2" → "/Game/Maps/Stage2"
	TArray<FName> Candidates;
	for (int32 i = StageVariantMin; i <= StageVariantMax; ++i)
	{
		const FName Candidate(*FString::Printf(TEXT("%s%d"), *StagePathPrefix, i));
		if (Candidate != InCurrentPathInState)
		{
			Candidates.Add(Candidate);
		}
	}

	// 후보가 없으면
	if (Candidates.Num() == 0)
	{
		return NAME_None;
	}

	const int32 Index = FMath::RandRange(0, Candidates.Num() - 1);	// 랜덤 선택
	return Candidates[Index];

	// 기존의 랜덤 선택 알고리즘 - 혹시 모를 무한 루프를 제거하기 위해 수정
	/*
	while (true)
	{
		const int32 Random = FMath::RandRange(StageVariantMin, StageVariantMax);
		const FName Candidate(*FString::Printf(TEXT("%s%d"), *StagePathPrefix, Random));
		if (Candidate != InCurrentPathInState)
		{
			return Candidate;
		}
	}
	// 아래 코드는 해당사항 거의 없음.
	return FName(*FString::Printf(TEXT("%s%d"), *StagePathPrefix, StageVariantMin));
	*/
}

void UCGStageStreamerSystem::DoStreamToStage(const FName& NextStagePath)
{
	if (GetWorld()->GetNetMode() != NM_DedicatedServer) return;
	if (bIsStreaming) return;
	bIsStreaming = true;
	PendingNextStagePath = NextStagePath;

	FLatentActionInfo Latent;
	Latent.CallbackTarget = this;
	Latent.ExecutionFunction = FName("OnLevelLoaded");	// 로그만 찍는 함수
	Latent.UUID = ++LatentUUID;
	Latent.Linkage = 0; // 명시해주면 안전
	UGameplayStatics::LoadStreamLevel(this, PendingNextStagePath, true, false, Latent);
	UE_LOG(LogTemp, Warning, TEXT("[Streamer] LoadStreamLevel called"));


	if (!InitPendingSubLevel())
	{
		return;
	}

	// 델리게이트 바인딩 (보이는 순간 텔레포트)
	PendingSubLevel->OnLevelLoaded.AddUniqueDynamic(this, &ThisClass::HandleLevelLoaded);	// OnLevelLoaded가 호출되면 HandleLevelLoaded도 호출되도록
}

bool UCGStageStreamerSystem::InitPendingSubLevel()
{
	// 포인터로 보관
	PendingSubLevel = FindLevelStreamingByAny(GetWorld(), PendingNextStagePath);

	// 포인터가 유효하지 않다면 리턴
	if (!PendingSubLevel.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[Streamer] No streaming object for %s"), *PendingNextStagePath.ToString());
		bIsStreaming = false;
		return false;
	}
	return true;
}

void UCGStageStreamerSystem::HandleAlreadyLoadedVisible()
{
	UE_LOG(LogTemp, Warning, TEXT("[Streamer] Target already loaded & visible — forcing flow"));
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		if (auto* StageState = GetWorld()->GetGameState<ACGStageStateBase>())
		{
			StageState->ResetStage();
		}
	}
	// 다음 틱에 '다음 레벨로 텔레포트 → 커밋/언로드' 한 번에 수행
	GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &ThisClass::TeleportThenCommit));
}

void UCGStageStreamerSystem::OnLevelLoaded()
{
	UE_LOG(LogTemp, Warning, TEXT("[Streamer] OnLevelLoaded fired (Pending=%s)"), *PendingNextStagePath.ToString());

}

void UCGStageStreamerSystem::OnLevelUnloaded()
{
	// 중복 호출 방지
	if (bUnloadHandle) return;
	bUnloadHandle = true;

	// 언로드 와치도그 타이머 클리어
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(UnloadWatchdog);
	}

	// 몬스터 스폰
	SpawnMonsterInNextStage();

	UE_LOG(LogTemp, Warning, TEXT("[Streamer] OnLevelUnloaded fired"));
	CurrentSubLevel = PendingSubLevel;
	if (CurrentSubLevel.IsValid())
	{
		if (ACGStageStateBase* StageState = GetStageState())
		{
			StageState->CurrentStagePath = PendingNextStagePath;
			UE_LOG(LogTemp, Log, TEXT("[OnLevelUnloaded] Stage State's CurrentStagePath: %s"), *StageState->CurrentStagePath.ToString());
		}
	}
	PendingSubLevel = nullptr;
	bIsStreaming = false;
	LastInteractor = nullptr;

}

void UCGStageStreamerSystem::SpawnMonsterInNextStage()
{
	FTimerHandle MonsterSpawnTimer;
	TWeakObjectPtr<ThisClass> WeakThis(this);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(MonsterSpawnTimer, FTimerDelegate::CreateLambda([WeakThis]()
			{
				if (WeakThis.IsValid())
				{
					UWorld* TimerWorld = WeakThis->GetWorld();
					if (TimerWorld)
					{
						AActor* FoundActor = UGameplayStatics::GetActorOfClass(TimerWorld, ACGMonsterSpawner::StaticClass());
						if (ACGMonsterSpawner* Spawner = Cast<ACGMonsterSpawner>(FoundActor))
						{
							Spawner->SpawnMonster();
						}
					}
				}
			}), 3.0f, false);
	}
}

void UCGStageStreamerSystem::HandleLevelLoaded()
{
	// 여기선 로드 됐는지만 확인. 텔포는 Shown에서.
	UE_LOG(LogTemp, Warning, TEXT("[Streamer] HandleLevelLoaded fired"));

	// 레벨 로드 스트리밍 종료
	bIsStreaming = false;
}

void UCGStageStreamerSystem::HandleLevelShown()
{
	UE_LOG(LogTemp, Warning, TEXT("[Streamer] HandleLevelShown fired"));

	if (PendingSubLevel->IsLevelLoaded() && PendingSubLevel->IsLevelVisible())
	{
		// 다음 스테이지로 선택된 레벨이 로드됐음을 체크하고 TeleportThenCommit() 진행
		HandleAlreadyLoadedVisible();
	}
}

void UCGStageStreamerSystem::TeleportThenCommit()
{
	if (GetWorld()->GetNetMode() != NM_DedicatedServer) return;

	// 텔레포트 및 언로드 스트리밍 Inflight 플래그 ON
	bIsStreaming = true;

	// 캐릭터는 다음 스테이지로 텔포
	TeleportInteractorToNewStage();

	// 이전 레벨 언로드 과정
	if (bUnloadPrevious)
	{
		bUnloadHandle = false;	// 멱등성 가드 초기화
		GetWorld()->GetTimerManager().ClearTimer(UnloadWatchdog);

		FLatentActionInfo Latent;
		Latent.CallbackTarget = this;
		Latent.ExecutionFunction = FName("OnLevelUnloaded");
		Latent.UUID = ++LatentUUID;
		Latent.Linkage = 0;

		UGameplayStatics::UnloadStreamLevel(this, PrevStagePath, Latent, false);
		
		// 언로드 와치도그	
		GetWorld()->GetTimerManager().SetTimer(UnloadWatchdog, this, &UCGStageStreamerSystem::OnLevelUnloaded, 3.0f, false);
		UE_LOG(LogTemp, Warning, TEXT("[Stage %s has Unloaded]"), *PrevStagePath.ToString());
		return; // 커밋은 OnLevelUnloaded에서
	}
}

APlayerStart* UCGStageStreamerSystem::FindPlayerStartIn(ULevel* InLevel) const
{
	if (!InLevel) return nullptr;

	// 1) 태그가 "StageSpawn"인 PlayerStart 우선(있으면 사용)
	static const FName StageSpawnTag(TEXT("StageSpawn"));
	for (AActor* Actor : InLevel->Actors)
	{
		if (Actor && Actor->IsA<APlayerStart>() && Actor->ActorHasTag(StageSpawnTag))
			return Cast<APlayerStart>(Actor);
	}
	// 2) 아무 PlayerStart나 첫 번째
	for (AActor* Actor : InLevel->Actors)
	{
		if (Actor && Actor->IsA<APlayerStart>())
			return Cast<APlayerStart>(Actor);
	}
	return nullptr;
}

void UCGStageStreamerSystem::TeleportInteractorToNewStage()
{
	UE_LOG(LogTemp, Warning, TEXT("[Streamer] Teleport Begin!"));
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{

		APawn* Pawn = LastInteractor.Get();
		if (!IsValid(Pawn)) return; // 누가 눌렀는지 없으면 스킵

		ULevelStreaming* LevelStreaming = PendingSubLevel.Get();
		if (!LevelStreaming) { UE_LOG(LogTemp, Error, TEXT("[Streamer] PendingLS null")); return; }

		ULevel* LoadedLevel = LevelStreaming->GetLoadedLevel();
		if (!LoadedLevel) { UE_LOG(LogTemp, Error, TEXT("[Streamer] LoadedLevel null")); return; }

		APlayerStart* PlayerStart = FindPlayerStartIn(LoadedLevel);

		// 스폰 트랜스폼 결정
		FTransform SpawnTM = FTransform::Identity;
		if (PlayerStart)
		{
			SpawnTM = PlayerStart->GetActorTransform(); // PlayerStart가 있으면 그 월드좌표로
		}
#if ENGINE_MAJOR_VERSION >= 5
		else
		{
			// 마지막 보루: 레벨 배치 트랜스폼의 원점
			SpawnTM = LevelStreaming->LevelTransform;
		}
#endif

		const FVector DestLocation = SpawnTM.GetLocation();
		const FRotator DestRotation = SpawnTM.Rotator();

		if (Pawn->TeleportTo(DestLocation, DestRotation, false, true))
		{
			if (auto* MovementComponent = Cast<UCharacterMovementComponent>(Pawn->GetMovementComponent()))
				MovementComponent->StopMovementImmediately();
			UE_LOG(LogTemp, Warning, TEXT("[Streamer] Teleported %s to %s"), *GetNameSafe(Pawn), *DestLocation.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[Streamer] Teleport failed for %s"), *GetNameSafe(Pawn));
		}
	}

}
