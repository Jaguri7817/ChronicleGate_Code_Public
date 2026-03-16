// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/CGMonsterSpawner.h"
#include "Engine/TargetPoint.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "CGStageStateBase.h"

// Sets default values
ACGMonsterSpawner::ACGMonsterSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;
}

void ACGMonsterSpawner::SpawnWave()
{
    if (!HasAuthority() || !MonsterClass) return;
    
    TArray<AActor*> Markers;
    CollectMarkers(Markers);
    if (Markers.Num() == 0)
    {
        return;
    }

    FRandomStream Rnd = bDeterministicRandom ? FRandomStream(RandomSeed) : FRandomStream(FMath::Rand());

    int32 N = Markers.Num();

    for (int32 i = 0; i < N; ++i)
    {
        AActor* Marker = Markers[i];
        UE_LOG(LogTemp, Log, TEXT("[Monster Spawn] Marker %d, Location: %s"), i, *Marker->GetActorLocation().ToString());
        for (int32 Try = 0; Try < MaxTriesPerSpawn; ++Try)
        {
            if (!Marker)
            {
                UE_LOG(LogTemp, Log, TEXT("[Monster Spawn] No Marker"));
                continue;
            }

            FVector Loc = PickRandomAround(Marker->GetActorLocation(), Rnd);
            
            // NavMesh 보정(실패하면 다른 시도)
            if (!FindValidNavLocation(Loc))
            {
                UE_LOG(LogTemp, Log, TEXT("[Monster Spawn] NavMesh Not Found"));
                continue;
            }

            /*
            // 자리 비었는지 충돌 체크
            if (!HasFreeSpaceAt(Loc))
                continue;

            */
            Loc.Z += 90.0f;
            const FRotator YawOnly(0.f, Rnd.FRandRange(-180.f, 180.f), 0.f);
            FActorSpawnParameters SpawnParameters;
            SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
            //SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn; // WARNING: 충돌이 발생해도 강제로 스폰 (디버깅용)
            APawn* Pawn = GetWorld()->SpawnActor<APawn>(MonsterClass, Loc, YawOnly, SpawnParameters);
            UE_LOG(LogTemp, Warning, TEXT("[Monster Spawn] Try=%d Result=%s Loc=%s"), Try, Pawn ? TEXT("OK") : TEXT("FAIL"), *Loc.ToString());

            if (Pawn) break;
        }
    }
}

void ACGMonsterSpawner::SpawnBoss()
{
    if (!HasAuthority() || !MonsterClass) return;

    TArray<AActor*> Markers;
    CollectMarkers(Markers);
    if (Markers.Num() == 0)
    {
        return;
    }

    FRandomStream Rnd = bDeterministicRandom ? FRandomStream(RandomSeed) : FRandomStream(FMath::Rand());

    int32 N = Markers.Num();

    for (int32 i = 0; i < N; ++i)
    {
        AActor* Marker = Markers[i];
        UE_LOG(LogTemp, Log, TEXT("[Monster Spawn] Marker %d, Location: %s"), i, *Marker->GetActorLocation().ToString());
        for (int32 Try = 0; Try < MaxTriesPerSpawn; ++Try)
        {
            if (!Marker)
            {
                UE_LOG(LogTemp, Log, TEXT("[Monster Spawn] No Marker"));
                continue;
            }

            FVector Loc = PickRandomAround(Marker->GetActorLocation(), Rnd);

            // NavMesh 보정(실패하면 다른 시도)
            if (!FindValidNavLocation(Loc))
            {
                UE_LOG(LogTemp, Log, TEXT("[Monster Spawn] NavMesh Not Found"));
                continue;
            }

            /*
            // 자리 비었는지 충돌 체크
            if (!HasFreeSpaceAt(Loc))
                continue;

            */
            Loc.Z += 90.0f;
            const FRotator YawOnly(0.f, Rnd.FRandRange(-180.f, 180.f), 0.f);
            FActorSpawnParameters SpawnParameters;
            SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
            //SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn; // WARNING: 충돌이 발생해도 강제로 스폰 (디버깅용)
            APawn* Pawn = GetWorld()->SpawnActor<APawn>(BossMonsterClass, Loc, YawOnly, SpawnParameters);
            UE_LOG(LogTemp, Warning, TEXT("[BossMonster Spawn] Try=%d Result=%s Loc=%s"), Try, Pawn ? TEXT("OK") : TEXT("FAIL"), *Loc.ToString());

            if (Pawn) break;
        }
    }
}

void ACGMonsterSpawner::SpawnMonster()
{
    if (!HasAuthority()) return;
    UE_LOG(LogTemp, Log, TEXT("[Monster Spawner] Spawn Monster"));

    ACGStageStateBase* StageState = GetWorld()->GetGameState<ACGStageStateBase>();
    if (StageState)
    {
        if (StageState->GetIsBossStage())
        {
            SpawnBoss();
        }
        else
        {
            SpawnWave();
        }
    }
}

// Called when the game starts or when spawned
void ACGMonsterSpawner::BeginPlay()
{
	Super::BeginPlay();
	if (!HasAuthority()) return;
    UE_LOG(LogTemp, Error, TEXT("[Monster Spawner] BeginPlay()"));
    
   
}

void ACGMonsterSpawner::CollectMarkers(TArray<AActor*>& Out) const
{
    Out.Reset();

    TArray<AActor*> AllOfTargetPoint;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATargetPoint::StaticClass(), AllOfTargetPoint);

    ULevel* CurrentLevel = GetLevel();

    for (AActor* TargetPoint : AllOfTargetPoint)
    {
        if (!TargetPoint) continue;
        if (TargetPoint->GetLevel() != CurrentLevel) continue;
        // 아래 코드는 특정 기믹을 수행하기 위한 타겟 포인트가 있을 시 사용
        // 사용 시 헤더 파일의 MarkerTag 주석 지우기
        //if (!MarkerTag.IsNone() && !TargetPoint->ActorHasTag(MarkerTag)) continue;

        Out.Add(TargetPoint);
    }

}

FVector ACGMonsterSpawner::PickRandomAround(const FVector& Center, FRandomStream& Rnd) const
{
    if (JitterRadius <= 0.f) return Center;
    // 아래 코드는 사용되지 않을 수도--------------------------

    const float Angle = Rnd.FRandRange(0.f, 2.f * PI);
    // 균등 면적 분포를 위해 sqrt 사용 (중요!)
    const float Radius = JitterRadius * FMath::Sqrt(Rnd.FRand());

    const FVector2D Off2(Radius * FMath::Cos(Angle), Radius * FMath::Sin(Angle));

    FVector Out = Center;
    Out.X += Off2.X;
    Out.Y += Off2.Y;
    return Out;
    //-----------------------------------------------------
}

bool ACGMonsterSpawner::FindValidNavLocation(FVector& InOut) const
{
    if (UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
    {
        FNavLocation NL;
        const FVector Extent(50.f, 50.f, NavQueryHalfHeight);
        if (Nav->ProjectPointToNavigation(InOut, NL, Extent))
        {
            InOut = NL.Location;
            return true;
        }
    }
    return false;
}

bool ACGMonsterSpawner::HasFreeSpaceAt(const FVector& Loc) const
{
    FHitResult Hit;
    FCollisionQueryParams P(SCENE_QUERY_STAT(SpawnCheck), false, nullptr);
    const FCollisionShape Shape = FCollisionShape::MakeSphere(CollisionCheckRadius);
    // Pawn 채널 기준, 필요시 커스텀 채널로 변경
    const bool bBlocked = GetWorld()->SweepSingleByChannel(Hit, Loc, Loc, FQuat::Identity, ECC_Pawn, Shape, P);
    return !bBlocked;
}

