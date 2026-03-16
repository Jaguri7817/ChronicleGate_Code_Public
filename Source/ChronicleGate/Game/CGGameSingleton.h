// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Character/CGStatData.h"
//#include "Engine/DataTable.h"		// UDataTable 사용
#include "CGGameSingleton.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(LogCGGameSingleton, Error, All);
/**
 * 
 */
UCLASS()
class CHRONICLEGATE_API UCGGameSingleton : public UObject
{
	GENERATED_BODY()
	
public:
	UCGGameSingleton();
	static UCGGameSingleton& Get();

// 캐릭터 스탯 데이터 Section
public:
	FORCEINLINE FCGBaseStat GetCharacterStat(int32 InLevel) const { return CharacterStatTable.IsValidIndex(InLevel - 1) ? CharacterStatTable[InLevel - 1] : FCGBaseStat(); }

	UPROPERTY()
	int32 CharacterMaxLevel;


private:
	TArray<FCGBaseStat> CharacterStatTable;

	
// 몬스터 스탯 데이터 Section
public:
	FORCEINLINE FCGBaseStat GetMonsterStat(int32 InLevel) const { return MonsterStatTable.IsValidIndex(InLevel - 1) ? MonsterStatTable[InLevel - 1] : FCGBaseStat(); }

	UPROPERTY()
	int32 MonsterMaxLevel;
private:
	TArray<FCGBaseStat> MonsterStatTable;
};
