// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Character/CGCharacterMonster.h"
#include "Character/CGCharacterBossMonster.h"
#include "CGMonsterSpawner.generated.h"

UCLASS()
class CHRONICLEGATE_API ACGMonsterSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	ACGMonsterSpawner();

    // 스폰할 몬스터 (Pawn/Character)
    UPROPERTY(EditAnywhere, Category = "Spawn")
    TSubclassOf<ACGCharacterMonster> MonsterClass;

    // 스폰할 보스 몬스터
    UPROPERTY(EditAnywhere, Category = "Spawn")
    TSubclassOf<ACGCharacterBossMonster> BossMonsterClass;


    // 한 번에 스폰될 최소/최대 수(랜덤)
    UPROPERTY(EditAnywhere, Category = "Spawn")
    int32 MinSpawnCount = 1;

    UPROPERTY(EditAnywhere, Category = "Spawn")
    int32 MaxSpawnCount = 3;

    // 레벨의 TargetPoint 중 이 태그가 붙은 것만 사용(비우면 전부)
    //UPROPERTY(EditAnywhere, Category = "Spawn")
    //FName MarkerTag;

    // 마커 주변으로 퍼뜨릴 반경(jitter). 0이면 정확히 마커 위치.
    UPROPERTY(EditAnywhere, Category = "Spawn")
    float JitterRadius = 0.f;

    // NavMesh 투영 시 위아래 탐색 높이
    UPROPERTY(EditAnywhere, Category = "Spawn")
    float NavQueryHalfHeight = 300.f;

    // 스폰 자리 충돌 검사 반경(대충 캡슐 반지름 느낌)
    UPROPERTY(EditAnywhere, Category = "Spawn")
    float CollisionCheckRadius = 34.f;

    // 자동 스폰할지/딜레이
    UPROPERTY(EditAnywhere, Category = "Spawn")
    bool bAutoSpawn = true;

    UPROPERTY(EditAnywhere, Category = "Spawn", meta = (EditCondition = "bAutoSpawn"))
    float InitialDelay = 0.f;

    // 한 개체를 배치하기 위해 시도할 최대 횟수(막혀있을 때 재시도)
    UPROPERTY(EditAnywhere, Category = "Spawn")
    int32 MaxTriesPerSpawn = 6;

    // (선택) 재현 가능한 랜덤
    UPROPERTY(EditAnywhere, Category = "Spawn")
    bool bDeterministicRandom = false;

    UPROPERTY(EditAnywhere, Category = "Spawn", meta = (EditCondition = "bDeterministicRandom"))
    int32 RandomSeed = 12345;

    UFUNCTION(BlueprintCallable, Category = "Spawn")
    void SpawnWave();
    void SpawnBoss();
    void SpawnMonster();

	
protected:
    virtual void BeginPlay() override;

private:
    void CollectMarkers(TArray<AActor*>& Out) const;
    FVector PickRandomAround(const FVector& Center, FRandomStream& Rnd) const;
    bool FindValidNavLocation(FVector& InOut) const;
    bool HasFreeSpaceAt(const FVector& Loc) const;

};
