// Fill out your copyright notice in the Description page of Project Settings.


#include "BlockChain/CGTokenAPIClient.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "JsonUtilities.h"

UCGTokenAPIClient::UCGTokenAPIClient()
{
}

void UCGTokenAPIClient::Init(const FString& InBaseUrl)
{
	BaseUrl = InBaseUrl;
}

bool UCGTokenAPIClient::ParseJson(TSharedPtr<class FJsonObject>& OutJson, const FString& Content) const
{
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
    return FJsonSerializer::Deserialize(Reader, OutJson) && OutJson.IsValid();
}

// 민트 =============================================================================================================================================

void UCGTokenAPIClient::MintToken(const FString& ToAddress, const FString& Amount)
{
    if (BaseUrl.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("[TokenAPI] BaseUrl is empty"));
        FTokenMintResult Result;
        Result.ErrorMessage = TEXT("BaseUrl is empty");
        OnMintCompleted.Broadcast(Result);
        return;
    }

    const FString Url = FString::Printf(TEXT("%s/tokens/mint"), *BaseUrl);

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(Url);
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    // Body
    TSharedPtr<FJsonObject> JsonBody = MakeShared<FJsonObject>();
    JsonBody->SetStringField(TEXT("toAddress"), ToAddress);
    JsonBody->SetStringField(TEXT("amount"), Amount);   // amount는 문자열로 전달

    FString BodyString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&BodyString);
    FJsonSerializer::Serialize(JsonBody.ToSharedRef(), Writer);

    Request->SetContentAsString(BodyString);
    Request->OnProcessRequestComplete().BindUObject(this, &UCGTokenAPIClient::OnMintTokenResponse);
    Request->ProcessRequest();

    // Latency 주의: 10~20초 걸릴 수 있음 → UI에서 로딩 표시 필요
    UE_LOG(LogTemp, Log, TEXT("[TokenAPI] MintToken request sent. Waiting for confirmation..."));
}

void UCGTokenAPIClient::OnMintTokenResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    FTokenMintResult Result;

    if (!bWasSuccessful || !Response.IsValid())
    {
        Result.ErrorMessage = TEXT("HTTP request failed");
        OnMintCompleted.Broadcast(Result);
        return;
    }

    const int32 StatusCode = Response->GetResponseCode();
    if (StatusCode != 200 && StatusCode != 201)
    {
        Result.ErrorMessage = FString::Printf(TEXT("Unexpected HTTP status code: %d"), StatusCode);
        OnMintCompleted.Broadcast(Result);
        return;
    }

    TSharedPtr<FJsonObject> RootJson;
    if (!ParseJson(RootJson, Response->GetContentAsString()))
    {
        Result.ErrorMessage = TEXT("Failed to parse JSON");
        OnMintCompleted.Broadcast(Result);
        return;
    }

    const bool bSuccessField = RootJson->GetBoolField(TEXT("success"));
    FString Status;
    RootJson->TryGetStringField(TEXT("status"), Status);

    if (!bSuccessField)
    {
        const TSharedPtr<FJsonObject>* ErrorObj;
        if (RootJson->TryGetObjectField(TEXT("error"), ErrorObj) && ErrorObj && ErrorObj->IsValid())
        {
            (*ErrorObj)->TryGetStringField(TEXT("code"), Result.ErrorCode);
            (*ErrorObj)->TryGetStringField(TEXT("message"), Result.ErrorMessage);
        }
        else
        {
            Result.ErrorMessage = TEXT("Mint failed: success=false");
        }

        OnMintCompleted.Broadcast(Result);
        return;
    }

    const TSharedPtr<FJsonObject>* DataObj;
    if (RootJson->TryGetObjectField(TEXT("data"), DataObj) && DataObj && DataObj->IsValid())
    {
        (*DataObj)->TryGetStringField(TEXT("txHash"), Result.TxHash);
        (*DataObj)->TryGetStringField(TEXT("amount"), Result.Amount);
        (*DataObj)->TryGetStringField(TEXT("recipient"), Result.Recipient);
        (*DataObj)->TryGetStringField(TEXT("explorerUrl"), Result.ExplorerUrl);
    }

    // 200/201 이더라도 txHash 없으면 실패로 간주
    if (Result.TxHash.IsEmpty())
    {
        Result.bSuccess = false;
        if (Result.ErrorMessage.IsEmpty())
        {
            Result.ErrorMessage = TEXT("Mint failed: txHash is empty");
        }
    }
    else
    {
        Result.bSuccess = true;
    }

    OnMintCompleted.Broadcast(Result);
}

// 소각 =============================================================================================================================================

void UCGTokenAPIClient::BurnToken(const FString& UserAddress, const FString& Amount)
{
    if (BaseUrl.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("[TokenAPI] BaseUrl is empty"));
        FTokenBurnResult Result;
        Result.ErrorMessage = TEXT("BaseUrl is empty");
        OnBurnCompleted.Broadcast(Result);
        return;
    }

    const FString Url = FString::Printf(TEXT("%s/tokens/burn"), *BaseUrl);

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(Url);
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    TSharedPtr<FJsonObject> JsonBody = MakeShared<FJsonObject>();
    JsonBody->SetStringField(TEXT("userAddress"), UserAddress);
    JsonBody->SetStringField(TEXT("amount"), Amount);

    FString BodyString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&BodyString);
    FJsonSerializer::Serialize(JsonBody.ToSharedRef(), Writer);

    Request->SetContentAsString(BodyString);
    Request->OnProcessRequestComplete().BindUObject(this, &UCGTokenAPIClient::OnBurnTokenResponse);
    Request->ProcessRequest();

    UE_LOG(LogTemp, Log, TEXT("[TokenAPI] BurnToken request sent."));
}

void UCGTokenAPIClient::OnBurnTokenResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    FTokenBurnResult Result;

    if (!bWasSuccessful || !Response.IsValid())
    {
        Result.ErrorMessage = TEXT("HTTP request failed");
        OnBurnCompleted.Broadcast(Result);
        return;
    }

    const int32 StatusCode = Response->GetResponseCode();
    if (StatusCode != 200)
    {
        Result.ErrorMessage = FString::Printf(TEXT("Unexpected HTTP status code: %d"), StatusCode);
        OnBurnCompleted.Broadcast(Result);
        return;
    }

    TSharedPtr<FJsonObject> RootJson;
    if (!ParseJson(RootJson, Response->GetContentAsString()))
    {
        Result.ErrorMessage = TEXT("Failed to parse JSON");
        OnBurnCompleted.Broadcast(Result);
        return;
    }

    const bool bSuccessField = RootJson->GetBoolField(TEXT("success"));
    if (!bSuccessField)
    {
        const TSharedPtr<FJsonObject>* ErrorObj;
        if (RootJson->TryGetObjectField(TEXT("error"), ErrorObj) && ErrorObj && ErrorObj->IsValid())
        {
            (*ErrorObj)->TryGetStringField(TEXT("code"), Result.ErrorCode);
            (*ErrorObj)->TryGetStringField(TEXT("message"), Result.ErrorMessage);
        }
        else
        {
            Result.ErrorMessage = TEXT("Burn failed: success=false");
        }

        OnBurnCompleted.Broadcast(Result);
        return;
    }

    const TSharedPtr<FJsonObject>* DataObj;
    if (RootJson->TryGetObjectField(TEXT("data"), DataObj) && DataObj && DataObj->IsValid())
    {
        (*DataObj)->TryGetStringField(TEXT("txHash"), Result.TxHash);
        (*DataObj)->TryGetStringField(TEXT("burnedAmount"), Result.BurnedAmount);
        (*DataObj)->TryGetStringField(TEXT("remainingAllowance"), Result.RemainingAllowance);
        (*DataObj)->TryGetStringField(TEXT("explorerUrl"), Result.ExplorerUrl);
    }

    // 마찬가지로 txHash 없으면 실패 처리
    if (Result.TxHash.IsEmpty())
    {
        Result.bSuccess = false;
        if (Result.ErrorMessage.IsEmpty())
        {
            Result.ErrorMessage = TEXT("Burn failed: txHash is empty");
        }
    }
    else
    {
        Result.bSuccess = true;
    }

    OnBurnCompleted.Broadcast(Result);
}

// 조회 =============================================================================================================================================

void UCGTokenAPIClient::GetBalance(const FString& Address)
{
    if (BaseUrl.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("[TokenAPI] BaseUrl is empty"));
        FTokenBalanceResult Result;
        Result.ErrorMessage = TEXT("BaseUrl is empty");
        OnBalanceCompleted.Broadcast(Result);
        return;
    }

    const FString Url = FString::Printf(TEXT("%s/tokens/balance/%s"), *BaseUrl, *Address);

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(Url);
    Request->SetVerb(TEXT("GET"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    Request->OnProcessRequestComplete().BindUObject(this, &UCGTokenAPIClient::OnBalanceResponse);
    Request->ProcessRequest();
}

void UCGTokenAPIClient::OnBalanceResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    FTokenBalanceResult Result;

    if (!bWasSuccessful || !Response.IsValid())
    {
        Result.ErrorMessage = TEXT("HTTP request failed");
        OnBalanceCompleted.Broadcast(Result);
        return;
    }

    const int32 StatusCode = Response->GetResponseCode();
    if (StatusCode != 200)
    {
        Result.ErrorMessage = FString::Printf(TEXT("Unexpected HTTP status code: %d"), StatusCode);
        OnBalanceCompleted.Broadcast(Result);
        return;
    }

    TSharedPtr<FJsonObject> RootJson;
    if (!ParseJson(RootJson, Response->GetContentAsString()))
    {
        Result.ErrorMessage = TEXT("Failed to parse JSON");
        OnBalanceCompleted.Broadcast(Result);
        return;
    }

    const bool bSuccessField = RootJson->GetBoolField(TEXT("success"));
    if (!bSuccessField)
    {
        const TSharedPtr<FJsonObject>* ErrorObj;
        if (RootJson->TryGetObjectField(TEXT("error"), ErrorObj) && ErrorObj && ErrorObj->IsValid())
        {
            (*ErrorObj)->TryGetStringField(TEXT("code"), Result.ErrorCode);
            (*ErrorObj)->TryGetStringField(TEXT("message"), Result.ErrorMessage);
        }
        else
        {
            Result.ErrorMessage = TEXT("Balance failed: success=false");
        }

        OnBalanceCompleted.Broadcast(Result);
        return;
    }

    const TSharedPtr<FJsonObject>* DataObj;
    if (RootJson->TryGetObjectField(TEXT("data"), DataObj) && DataObj && DataObj->IsValid())
    {
        (*DataObj)->TryGetStringField(TEXT("address"), Result.Address);
        (*DataObj)->TryGetStringField(TEXT("balance"), Result.Balance);
        (*DataObj)->TryGetStringField(TEXT("symbol"), Result.Symbol);
    }

    Result.bSuccess = true;
    OnBalanceCompleted.Broadcast(Result);
}
