// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/CGGameSingleton.h"


DEFINE_LOG_CATEGORY(LogCGGameSingleton);

UCGGameSingleton::UCGGameSingleton()
{
	UE_LOG(LogTemp, Log, TEXT("SingleTon Start"));
	// 캐릭터 Section ===============================================================================================================================================
	static ConstructorHelpers::FObjectFinder<UDataTable> CharacterDataTableRef(TEXT("/Script/Engine.DataTable'/Game/ChronicleGate/Data/CGBaseStatDataTable.CGBaseStatDataTable'"));
	if (nullptr != CharacterDataTableRef.Object)
	{
		const UDataTable* CharacterDataTable = CharacterDataTableRef.Object;
		check(CharacterDataTable->GetRowMap().Num() > 0);

		TArray<uint8*> CharacterValueArray;
		CharacterDataTable->GetRowMap().GenerateValueArray(CharacterValueArray);
		Algo::Transform(CharacterValueArray, CharacterStatTable,
			[](uint8* Value)
			{
				return *reinterpret_cast<FCGBaseStat*>(Value);
			}
		);
	}

	CharacterMaxLevel = CharacterStatTable.Num();
	ensure(CharacterMaxLevel > 0);

	// 몬스터 Section ===============================================================================================================================================
	static ConstructorHelpers::FObjectFinder<UDataTable> MonsterDataTableRef(TEXT("/Script/Engine.DataTable'/Game/ChronicleGate/Data/CGMonsterStatDataTable.CGMonsterStatDataTable'"));
	if (nullptr != MonsterDataTableRef.Object)
	{
		const UDataTable* MonsterDataTable = MonsterDataTableRef.Object;
		check(MonsterDataTable->GetRowMap().Num() > 0);

		TArray<uint8*> MonsterValueArray;
		MonsterDataTable->GetRowMap().GenerateValueArray(MonsterValueArray);
		Algo::Transform(MonsterValueArray, MonsterStatTable,
			[](uint8* Value)
			{
				return *reinterpret_cast<FCGBaseStat*>(Value);
			}
		);
	}

	MonsterMaxLevel = MonsterStatTable.Num();
	ensure(MonsterMaxLevel > 0);
}


UCGGameSingleton& UCGGameSingleton::Get()
{
	UCGGameSingleton* Singleton = CastChecked<UCGGameSingleton>(GEngine->GameSingleton);
	if (Singleton)
	{
		return *Singleton;
	}

	UE_LOG(LogCGGameSingleton, Error, TEXT("Invalid Game Singleton"));
	return *NewObject<UCGGameSingleton>();
}