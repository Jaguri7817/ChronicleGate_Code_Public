// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "BlockChain/CGRewardType.h"
#include "CGSkinListItemObject.generated.h"

/**
 * 
 */
UCLASS()
class CHRONICLEGATE_API UCGSkinListItemObject : public UObject
{
	GENERATED_BODY()
	
public:
    UPROPERTY(BlueprintReadOnly)
    FCGNFTSkinInfo SkinInfo;

    void Init(const FCGNFTSkinInfo& InInfo)
    {
        SkinInfo = InInfo;
    }
	
	
};
