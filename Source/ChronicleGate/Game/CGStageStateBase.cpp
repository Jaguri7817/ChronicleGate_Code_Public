

#include "Game/CGStageStateBase.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetSystemLibrary.h"
#include "CGMonsterSpawner.h"
#include "CGGameMode.h"
#include "Character/CGCharacterMonster.h"
#include "BlockChain/CGBackendSubsystem.h"
#include "Equipment/CGBossRewardDataAsset.h"
#include "Blueprint/UserWidget.h"
#include "Controller/CGCharacterPlayerController.h"
#include "CGStageStreamerSystem.h"

ACGStageStateBase::ACGStageStateBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bIsBossStage = false;

	static ConstructorHelpers::FObjectFinder<UCGBossRewardDataAsset> BossRewardDataRef(TEXT("/Script/ChronicleGate.CGBossRewardDataAsset'/Game/ChronicleGate/Data/BossRewardData.BossRewardData'"));
	if (BossRewardDataRef.Object)
	{
		BossRewardItemCode = BossRewardDataRef.Object;
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> StageClearWidgetRef(TEXT("/Game/HUD/WBP_StageClear.WBP_StageClear_C"));
	if (StageClearWidgetRef.Class)
	{
		StageClearWidgetClass = StageClearWidgetRef.Class;
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> ReturnToLobbyWidgetRef(TEXT("/Game/HUD/WBP_ReturnToLobby.WBP_ReturnToLobby_C"));
	if (ReturnToLobbyWidgetRef.Class)
	{
		ReturnToLobbyWidgetClass = ReturnToLobbyWidgetRef.Class;
	}
}

void ACGStageStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACGStageStateBase, bStageCleared);
	DOREPLIFETIME(ACGStageStateBase, AliveMonsterCount);
}

void ACGStageStateBase::ResetStage()
{
	if (!HasAuthority()) return;

	bStageCleared = false;
	AliveMonsterCount = 0;
	AliveMonsters.Empty();

	++StageNum;
	
	bIsBossStage = (StageNum == MaxStageNum);
	UE_LOG(LogTemp, Warning, TEXT("[StageState] ResetStage: StageNum=%d, bIsBossStage=%d"), StageNum, bIsBossStage);

	ForceNetUpdate();
	OnStageClearedChanged.Broadcast(bStageCleared);

#if WITH_EDITOR
	UKismetSystemLibrary::PrintString(this, TEXT("Stage Reset"), true, true, FLinearColor::Yellow, 1.2f);
#endif
}

// CharacterMontster.cpp BeginPlay()에서 추가됨
void ACGStageStateBase::RegisterMonster(AActor* Monster)
{
	if (!HasAuthority() || !IsValid(Monster)) return;	// 실행 주체가 서버도 아니고 유효하지 않다면 리턴
	if (AliveMonsters.Contains(Monster)) return;		// 똑같은 객체가 있으면 리턴

	AliveMonsters.Add(Monster);
	++AliveMonsterCount;

	if (bIsBossStage)
	{
		ACGCharacterBossMonster* Boss = Cast<ACGCharacterBossMonster>(Monster);
		if (Boss)
		{
			CurrentBossName = Boss->GetBossName();
			UE_LOG(LogTemp, Warning, TEXT("[StageState] Boss Registered, EventId=%s"), *CurrentBossName.ToString());
		}
	}

	// 새 몬스터가 생겼으면 클리어 플래그는 내려가 있어야 함
	if (bStageCleared)
	{
		bStageCleared = false;
		OnStageClearedChanged.Broadcast(false);
	}

	ForceNetUpdate();
}

// CharacterMonster.cpp SetDead()에서 호출됨
void ACGStageStateBase::NotifyMonsterDied(AActor* Monster)
{
	if (!HasAuthority() || !IsValid(Monster)) return;	// 실행 주체가 서버도 아니고 유효하지 않다면 리턴
	if (!AliveMonsters.Contains(Monster)) return;		// 이미 제거됐거나 스테이지 소속이 아니면 리턴

	AliveMonsters.Remove(Monster);
	AliveMonsterCount = FMath::Max(0, AliveMonsterCount - 1);

	if (AliveMonsterCount == 0 && !bStageCleared)
	{
		StageClear();
	}

	ForceNetUpdate();
}

void ACGStageStateBase::OnPlayerDead()
{
	UE_LOG(LogTemp, Warning, TEXT("[StageState] Travel To Lobby"));
	if (!HasAuthority()) return;
	ACGGameMode* GameMode = GetWorld()->GetAuthGameMode<ACGGameMode>();
	if (!GameMode) return;
	if (GameMode->IsTraveling() || bPendingTravel) return;

	bPendingTravel = true;
	FTimerHandle TravelTimer;
	GetWorld()->GetTimerManager().SetTimer(TravelTimer, this, &ACGStageStateBase::DoTravelToLobby, 5.0f, false);

}

void ACGStageStateBase::DoTravelToLobby()
{
	bPendingTravel = false;
	if (!HasAuthority()) return;

	ACGGameMode* GameMode = GetWorld()->GetAuthGameMode<ACGGameMode>();
	if (!GameMode) return;
	if (GameMode->IsTraveling()) return;

	GameMode->TravelToLobby();
}

void ACGStageStateBase::StageClear()
{
	// 안전장치 ------------------
	if (!HasAuthority()) return;
	if (bStageCleared) return;	
	// --------------------------

	bStageCleared = true;
	OnStageClearedChanged.Broadcast(true);
	OnRep_StageCleared();

#if WITH_EDITOR
	UKismetSystemLibrary::PrintString(this, TEXT("Stage Cleared!"), true, true, FLinearColor::Green, 1.5f);
#endif

	if (bIsBossStage)
	{
		HandleBossStageClear();
		return;
	}

	// 스테이지가 클리어되면 다음 스테이지 랜덤 선택 -> 선택된 레벨 로드 -> Inflight 플래그 OFF
	UCGStageStreamerSystem* StageStreamer = GetWorld()->GetSubsystem<UCGStageStreamerSystem>();
	if (!StageStreamer) { UE_LOG(LogTemp, Log, TEXT("[StageState] StageStreamer is Null")); }

	StageStreamer->Server_RequestStageStream(CurrentStagePath);
	

}

void ACGStageStateBase::HandleBossStageClear()
{
	if (!bStageCleared || !bIsBossStage) return;
	if (CurrentBossName == NAME_None) return;
	if (!BossRewardItemCode)
	{
		UE_LOG(LogTemp, Error, TEXT("[StageState] BossRewardItemCode DataAsset is NULL"));
		return;
	}

	// 1) 보상 요청
	if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
	{
		UCGBackendSubsystem* Backend = GameInstance->GetSubsystem<UCGBackendSubsystem>();
		if (Backend)
		{
			const FString WalletAddress = Backend->GetWalletAddress();
			const FString GameEventId = FString::Printf(TEXT("%s_%s"), *WalletAddress, *CurrentBossName.ToString());
			const FName ItemCode = BossRewardItemCode->PickRandomItemCode();
			Backend->RequestDungeonClearReward(GameEventId, ItemCode.ToString());

			if (HasAuthority())
			{
				for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
				{
					if (ACGCharacterPlayerController* CGPlayerController = Cast<ACGCharacterPlayerController>(*Iterator))
					{
						CGPlayerController->ClientRPC_NotifyBossReward(ItemCode);
					}
				}
			}
		}
	}

	// 2) 10초 뒤 로비로 이동 예약
	const float ReturnDelay = 10.0f;
	GetWorldTimerManager().SetTimer(BossStageClearTimerHandle, this, &ACGStageStateBase::DoTravelToLobby, ReturnDelay, false);

	// 3) 클라에게 "n초 후 로비로 귀환" 위젯 띄우기
	MulticastRPC_ShowReturnToLobbyWidget(ReturnDelay);
}

void ACGStageStateBase::OnRep_StageCleared()
{
	if (!bStageCleared) return;

	// 클라에서 UI 갱신용
	OnStageClearedChanged.Broadcast(bStageCleared);

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!PlayerController || !PlayerController->IsLocalController()) return;

	if (StageClearWidgetClass)
	{
		UUserWidget* StageClearWidget = CreateWidget<UUserWidget>(PlayerController, StageClearWidgetClass);
		if (StageClearWidget)
		{
			StageClearWidget->AddToViewport();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No StageClearWidgetClass"));
	}
}

void ACGStageStateBase::MulticastRPC_ShowReturnToLobbyWidget_Implementation(float InDelaySeconds)
{
	// 서버에서 UI 안 띄움
	if (IsRunningDedicatedServer()) return;

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!PlayerController || !PlayerController->IsLocalController()) return;

	if (!ReturnToLobbyWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[StageState] ReturnToLobbyWidgetClass is null"));
		return;
	}
	if (!ReturnToLobbyWidgetInstance)
	{
		ReturnToLobbyWidgetInstance = CreateWidget<UUserWidget>(PlayerController, ReturnToLobbyWidgetClass);
		if (ReturnToLobbyWidgetInstance)
		{
			ReturnToLobbyWidgetInstance->AddToViewport();
			UE_LOG(LogTemp, Log, TEXT("[StageState] Show ReturnToLobbyWidget (%.1f sec)"), InDelaySeconds);

			// 남은 초를 위젯에 넘기고 싶으면:
			// - 위젯 BP에 int 변수 RemainingSeconds 만들어서
			// - C++로 캐스팅해서 세팅하거나
			// - BP에서 "10초 후, 로비로 귀환합니다." 고정 텍스트로 둬도 됨 <- 현재 세팅
		}
	}
}
