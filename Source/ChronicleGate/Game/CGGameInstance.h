// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BlockChain/CGRewardType.h"
#include "ApiConfig/ApiConfig.h"
#include "CGGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class CHRONICLEGATE_API UCGGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UCGGameInstance();

	virtual void Init() override;
	virtual void Shutdown() override;

	FORCEINLINE const FString& GetWalletAddress() const { return UserWalletAddress; }
	FORCEINLINE void SetWalletAddress(const FString& InWallet) { UserWalletAddress = InWallet; }
	FORCEINLINE const FString& GetServerAddress() const { return ServerAddress; }
	void SetSelectedSkin(const FCGNFTSkinInfo& InSkin);
	bool GetSelectedSkin(FCGNFTSkinInfo& OutSkin) const;

protected:
	FString UserWalletAddress;

	FString ServerAddress = ApiConfig::LocalAddress; // 로컬 테스트 주소
	//FString ServerAddress = ApiConfig::AzureServerAddress; // 엣져 테스트 주소

	UPROPERTY()
	FCGNFTSkinInfo SelectedSkin;

	bool bHasSelectedSkin = false;
};
