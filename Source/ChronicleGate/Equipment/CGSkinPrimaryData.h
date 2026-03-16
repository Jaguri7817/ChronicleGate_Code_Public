// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CGSkinPrimaryData.generated.h"

/**
 * 
 */
UCLASS()
class CHRONICLEGATE_API UCGSkinPrimaryData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UCGSkinPrimaryData();

	UPROPERTY(EditAnywhere, Category = "SkinData")
	FString ItemCode;
	UPROPERTY(EditAnywhere, Category = "SkinData")
	FName SkinName;
	UPROPERTY(EditAnywhere, Category = "SkinData")
	TSoftObjectPtr<USkeletalMesh> SkinMesh;


	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(TEXT("Skin"), FName(*ItemCode));
	}
	
	
};
