// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CGRewardType.h"
#include "Interfaces/IHttpRequest.h"       // IHttpRequest
#include "Interfaces/IHttpResponse.h"      // IHttpResponse
#include "CGRewardAPIClient.generated.h"



DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPostRewardCompleted, bool /*bSuccess*/, const FString& /*ErrorMessage*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGetRewardsCompleted, bool /*bSuccess*/, const TArray<FCGRewardRecord>& /*Rewards*/);
/**
 * 
 */
UCLASS()
class CHRONICLEGATE_API UCGRewardAPIClient : public UObject
{
	GENERATED_BODY()
	
public:
	void Init(const FString& InBaseUrl);
	
	// 던전 클리어 후 보상 요청
	void PostRewardRequest(const FString& InWalletAddress, const FString& InGameEventId, const FString& InItemCode);

	// 유저의 보상 내역 조회
	void GetRewards(const FString& WalletAddress);

	// 델리게이트들
	
	FOnPostRewardCompleted OnPostRewardCompleted;
	
	FOnGetRewardsCompleted OnGetRewardsCompleted;
	
private:
	FString BaseUrl;

	void HandlePostRewardResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void HandleGetRewardsResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

};
