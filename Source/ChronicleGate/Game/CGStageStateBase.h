#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CGStageStateBase.generated.h"

// 클리어 상태 변경 브로드캐스트 (UI용)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStageClearedChanged, bool, bCleared);

UCLASS()
class CHRONICLEGATE_API ACGStageStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	ACGStageStateBase();

	// 조회용 게터
	UFUNCTION(BlueprintPure, Category = "Stage")
	bool IsStageCleared() const { return bStageCleared; }

	// 살아남은 몬스터 마릿수 게터
	UFUNCTION(BlueprintPure, Category = "Stage")
	int32 GetAliveCount() const { return AliveMonsterCount; }

	// 다음 스테이지 시작 시 서버에서 호출
	UFUNCTION(BlueprintCallable, Category = "Stage")
	void ResetStage();

	// === 서버 전용 API (스포너/몬스터가 호출) ===
	// Monster, BossMonster의 BeginPlay()에서 호출
	UFUNCTION(BlueprintCallable, Category = "Stage")
	void RegisterMonster(AActor* Monster);

	// Monster, BossMonster의 SetDead()에서 호출
	UFUNCTION(BlueprintCallable, Category = "Stage")
	void NotifyMonsterDied(AActor* Monster);

	// UI 바인딩 (BP에서 OnStageClearedChanged.Add)
	UPROPERTY(BlueprintAssignable, Category = "Stage")
	FOnStageClearedChanged OnStageClearedChanged;

	// 현재 스테이지 경로(패키지 경로)
	UPROPERTY(VisibleInstanceOnly) FName CurrentStagePath = FName(TEXT("/Game/Maps/Stage1"));
	
	// 플레이어 사망 시 로비로 이동하는 함수
	void OnPlayerDead();
	void DoTravelToLobby();
	uint8 bPendingTravel = false;

	// 보스 스테이지
	int32 MaxStageNum = 4;
	uint8 bIsBossStage : 1;
	FORCEINLINE bool GetIsBossStage() { return bIsBossStage; }
	FORCEINLINE int32 GetMaxStageNum() { return MaxStageNum; }
	void StageClear();
	void HandleBossStageClear();
	FName CurrentBossName = NAME_None;
	FTimerHandle BossStageClearTimerHandle;
	
	// 보상
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BossReward")
	TObjectPtr<class UCGBossRewardDataAsset> BossRewardItemCode;

protected:
	// 복제: 스테이지 클리어 여부
	UPROPERTY(ReplicatedUsing = OnRep_StageCleared, VisibleAnywhere, Category = "Stage")
	bool bStageCleared = true;

	// 복제: 현재 생존 몬스터 수(간단 모니터링/UI 표시용)
	UPROPERTY(Replicated, VisibleAnywhere, Category = "Stage")
	int32 AliveMonsterCount = 0;

	// [약한 포인터] 서버 전용: 중복 방지/정합성
	UPROPERTY()
	TSet<TWeakObjectPtr<AActor>> AliveMonsters;

	UFUNCTION()
	void OnRep_StageCleared();
	int32 StageNum = 0;

public:
	FORCEINLINE int32 GetStageNum() { return StageNum; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

// UI Section
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> StageClearWidgetClass;

	// 위젯 클래스/인스턴스
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> ReturnToLobbyWidgetClass;

	UPROPERTY()
	TObjectPtr<class UUserWidget> ReturnToLobbyWidgetInstance;

	// 멀티캐스트 RPC
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_ShowReturnToLobbyWidget(float InDelaySeconds);
};
