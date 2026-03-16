// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CGGameMode.generated.h"

/**
 * 
 */
UCLASS()
class CHRONICLEGATE_API ACGGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ACGGameMode();
	
	UFUNCTION(BlueprintCallable) void TravelToDungeon();
	UFUNCTION(BlueprintCallable) void TravelToLobby();
	
	FORCEINLINE bool IsTraveling() const { return bIsTraveling;}

private:
	bool bIsTraveling = false;

	const FString PersistentLevelName = TEXT("Stage");
	UFUNCTION() virtual void PostSeamlessTravel() override;
	UFUNCTION() void StageLoadAfterTravel();
	const FName FirstStage = FName(TEXT("/Game/Maps/Stage1"));
};
