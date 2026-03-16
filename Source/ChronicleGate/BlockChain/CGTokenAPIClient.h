// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CGTokenType.h"
#include "HttpModule.h"
#include "Dom/JsonObject.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "CGTokenAPIClient.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTokenMintCompleted, const FTokenMintResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTokenBurnCompleted, const FTokenBurnResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTokenBalanceCompleted, const FTokenBalanceResult&, Result);


UCLASS()
class CHRONICLEGATE_API UCGTokenAPIClient : public UObject
{
	GENERATED_BODY()
	
public:
    UCGTokenAPIClient();

    void Init(const FString& InBaseUrl);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TokenAPI")
    FString BaseUrl;

    UFUNCTION(BlueprintCallable, Category = "TokenAPI")
    void MintToken(const FString& ToAddress, const FString& Amount);

    UFUNCTION(BlueprintCallable, Category = "TokenAPI")
    void BurnToken(const FString& UserAddress, const FString& Amount);

    UFUNCTION(BlueprintCallable, Category = "TokenAPI")
    void GetBalance(const FString& Address);

    UPROPERTY(BlueprintAssignable, Category = "TokenAPI")
    FOnTokenMintCompleted OnMintCompleted;

    UPROPERTY(BlueprintAssignable, Category = "TokenAPI")
    FOnTokenBurnCompleted OnBurnCompleted;

    UPROPERTY(BlueprintAssignable, Category = "TokenAPI")
    FOnTokenBalanceCompleted OnBalanceCompleted;

private:
    void OnMintTokenResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
    void OnBurnTokenResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
    void OnBalanceResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

    bool ParseJson(TSharedPtr<class FJsonObject>& OutJson, const FString& Content) const;
	
	
};
