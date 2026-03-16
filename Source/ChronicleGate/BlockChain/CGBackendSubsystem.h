// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CGRewardType.h"
#include "CGTokenType.h"
#include "CGBackendSubsystem.generated.h"


DECLARE_MULTICAST_DELEGATE_OneParam(FOnRewardsUpdated, const TArray<FCGRewardRecord>&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnNFTSkinsUpdated, const TArray<FCGNFTSkinInfo>&);
/**
 * 
 */
UCLASS()
class CHRONICLEGATE_API UCGBackendSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;


// NFT Skin Section
    // 유저 지갑(or ID) 설정
    FORCEINLINE void SetWalletAddress(const FString& InWalletAddress) { WalletAddress = InWalletAddress; }
    FORCEINLINE FString GetWalletAddress() { return WalletAddress; }
    // 1) 던전 클리어 후 보상 요청
    void RequestDungeonClearReward(const FString& InDungeonId, const FString& InItemCode);

    // 2) 현재 유저 보상 내역 조회 → 내부 캐시 업데이트
    void RefreshRewards();

    // 3) UI에서 장착 가능한 NFT 스킨 리스트 가져가기
    FORCEINLINE const TArray<FCGNFTSkinInfo>& GetOwnedNFTSkins() const { return OwnedNFTSkins; }

    // UI에 브로드캐스트할 델리게이트
    FOnRewardsUpdated OnRewardsUpdated;
    FOnNFTSkinsUpdated OnNFTSkinsUpdated;

private:
    UPROPERTY()
    TObjectPtr<class UCGRewardAPIClient> RewardAPIClient;
    

    FString WalletAddress;

    TArray<FCGRewardRecord> CachedRewards;
    TArray<FCGNFTSkinInfo> OwnedNFTSkins;

    // ApiClient 델리게이트 핸들러
    void HandlePostRewardCompleted(bool bSuccess, const FString& ErrorMessage);
    void HandleGetRewardsCompleted(bool bSuccess, const TArray<FCGRewardRecord>& Rewards);

    // 보상 -> NFT 스킨 변환 (COMPLETED + nft != null 필터링)
    void RebuildOwnedNFTSkinsFromRewards();
	

// Token Section
public:
    // ------ 외부에서 부를 토큰 관련 함수 래핑 ------
    UFUNCTION(BlueprintCallable, Category = "Backend|Token")
    void RequestMintToken(const FString& ToAddress, const FString& Amount);

    UFUNCTION(BlueprintCallable, Category = "Backend|Token")
    void RequestBurnToken(const FString& UserAddress, const FString& Amount);

    UFUNCTION(BlueprintCallable, Category = "Backend|Token")
    void RequestTokenBalance(const FString& Address);

private:
    UPROPERTY()
    TObjectPtr<class UCGTokenAPIClient> TokenAPIClient;

    // ------ 토큰 콜백 처리용 ------
    UFUNCTION()
    void HandleMintResult(const FTokenMintResult& Result);

    UFUNCTION()
    void HandleBurnResult(const FTokenBurnResult& Result);

    UFUNCTION()
    void HandleBalanceResult(const FTokenBalanceResult& Result);
};
