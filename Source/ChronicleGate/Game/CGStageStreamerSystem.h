// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CGStageStreamerSystem.generated.h"

/**
 * 
 */
class APlayerStart;   // 추가
class ULevel;         // 추가
class ACGStageStateBase;

UCLASS(Config = Game)
class CHRONICLEGATE_API UCGStageStreamerSystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	UCGStageStreamerSystem();

    // ====== 설정 ======
    // "/Game/Maps/Stage" (Stage, Stage1, Stage2...)
    UPROPERTY(EditAnywhere, Config, Category = "Stage")
    FString StagePathPrefix = TEXT("/Game/Maps/Stage");

    // 보스 스테이지 경로
    UPROPERTY(EditAnywhere, Category = "Stage")
    FName BossStagePath = FName(TEXT("/Game/Maps/BossStage1"));

    // 스테이지의 최소 번호
    UPROPERTY(EditAnywhere, Config, Category = "Stage")
    int32 StageVariantMin = 1;
    // 스테이지의 최대 번호
    UPROPERTY(EditAnywhere, Config, Category = "Stage")
    int32 StageVariantMax = 3;

    // 이전 레벨 언로드 플래그
    UPROPERTY(EditAnywhere, Config, Category = "Stage")
    bool bUnloadPrevious = true;

    // bIsStreaming 게터
    FORCEINLINE bool IsStageStreaming() const { return bIsStreaming; }

    // StageState 게터
    ACGStageStateBase* GetStageState() const;


    void Server_RequestStageStream(const FName& InCurrentPathInState);
	
protected:
    // 현재 스테이지 경로(패키지 경로)
    //UPROPERTY(VisibleInstanceOnly) FName CurrentStagePath;
    // 다음 스테이지 경로(패키지 경로)
    UPROPERTY(VisibleInstanceOnly) FName PendingNextStagePath;


    UPROPERTY(VisibleAnywhere) bool bIsStreaming = false;
    // ====== 랜덤/스트리밍 ======
    FName ChooseNextStage(const FName& InCurrentPathInState) const;   // 다음 스테이지 선택
public:  // 던전 트래블 시 스테이지1을 로딩하기 위해
    void  DoStreamToStage(const FName& NextStagePath);          // 다음 스테이지로 스트리밍
private:
    bool InitPendingSubLevel();                                 // DoStreamToStage 모듈화 함수
    void HandleAlreadyLoadedVisible();                          // DoStreamToStage 모듈화 함수
    int32 LatentUUID = 1001;
    UFUNCTION() void OnLevelLoaded();                           // 다음 스테이지 로드되면
    UFUNCTION() void OnLevelUnloaded();                         // 언로드
    void SpawnMonsterInNextStage();                             // 다음 스테이지 텔레포트 후 진행될 몬스터 스폰 로직

    // 현재 활성 서브레벨
    UPROPERTY() TWeakObjectPtr<ULevelStreaming> CurrentSubLevel;

    // 이번에 로드할 서브레벨
    UPROPERTY() TWeakObjectPtr<ULevelStreaming> PendingSubLevel;

    UFUNCTION() void HandleLevelLoaded();   // 로드 완료됐음을 로그로 확인해보기 위헤
public:
    UFUNCTION() void HandleLevelShown();    // 텔레포트 & 언로드 시작
protected:
    UFUNCTION() void TeleportThenCommit();  // 텔레포트 후 저장
    UPROPERTY() FName LastCommittedStagePath; // 항상 "/Game/Maps/StageN" 형태

    // ========= 텔레포트용 멤버 =========
    UPROPERTY()
    TWeakObjectPtr<APawn> LastInteractor;     // E를 누른 주인공
public:
    FORCEINLINE void SetLastInteractor(APawn* InPawn) { LastInteractor = InPawn; }
protected:

    UPROPERTY(VisibleInstanceOnly)
    FName PrevStagePath;                       // 이전 스테이지 패키지 경로

    // 헬퍼
    APlayerStart* FindPlayerStartIn(ULevel* InLevel) const;
    void TeleportInteractorToNewStage();

private:
    FTimerHandle UnloadWatchdog;
    uint8 bUnloadHandle;
	
};
