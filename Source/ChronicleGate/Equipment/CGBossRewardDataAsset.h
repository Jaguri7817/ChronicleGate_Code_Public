// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CGBossRewardDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class CHRONICLEGATE_API UCGBossRewardDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	FName PickRandomItemCode() const
	{
		if (BossRewardItemCode.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("[StageState] No BossRewardItemCode set"));
			return NAME_None;
		}
		const int32 Index = FMath::RandRange(0, BossRewardItemCode.Num() - 1);
		return BossRewardItemCode[Index];
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossReward")
	TArray<FName> BossRewardItemCode;
	
	
};
