// Fill out your copyright notice in the Description page of Project Settings.


#include "BlockChain/CGRewardAPIClient.h"
#include "HttpModule.h"                    // FHttpModule
#include "Dom/JsonObject.h"                // TSharedPtr<FJsonObject>
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "JsonUtilities.h"                 // FJsonObjectConverter 등 쓸 때



void UCGRewardAPIClient::Init(const FString& InBaseUrl)
{
	BaseUrl = InBaseUrl;
}


void UCGRewardAPIClient::PostRewardRequest(const FString& InWalletAddress, const FString& InGameEventId, const FString& InItemCode)
{
    // 기본적인 유효성 체크
    if (InWalletAddress.IsEmpty() || InGameEventId.IsEmpty())
    {
        if (OnPostRewardCompleted.IsBound())
        {
            OnPostRewardCompleted.Broadcast(false, TEXT("WalletAddress or GameEventId is empty."));
        }
        return;
    }

    // HTTP 리퀘스트 생성
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    // URL: BaseUrl + "/rewards/request"
    const FString Url = FString::Printf(TEXT("%s/rewards/request"), *BaseUrl);  // BaseUrl 끝에 / 안 붙어있다는 전제
    Request->SetURL(Url);

    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    // JSON 바디 생성
    TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();
    JsonObject->SetStringField(TEXT("walletAddress"), InWalletAddress);
    JsonObject->SetStringField(TEXT("gameEventId"), InGameEventId);
    JsonObject->SetStringField(TEXT("itemCode"), InItemCode);

    FString BodyString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&BodyString);
    FJsonSerializer::Serialize(JsonObject, Writer);

    Request->SetContentAsString(BodyString);

    // 콜백 바인딩
    Request->OnProcessRequestComplete().BindUObject(this, &UCGRewardAPIClient::HandlePostRewardResponse);

    // 요청 보내기
    Request->ProcessRequest();
}

void UCGRewardAPIClient::GetRewards(const FString& InWalletAddress)
{
    if (InWalletAddress.IsEmpty())
    {
        if (OnGetRewardsCompleted.IsBound())
        {
            TArray<FCGRewardRecord> Empty;
            OnGetRewardsCompleted.Broadcast(false, Empty);
        }
        return;
    }

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    // walletAddress 쿼리로 붙이기 (0x... 는 URL-safe라 인코딩 생략해도 됨)
    const FString Url = FString::Printf(TEXT("%s/rewards/check?walletAddress=%s"), *BaseUrl, *InWalletAddress);

    Request->SetURL(Url);
    Request->SetVerb(TEXT("GET"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    Request->OnProcessRequestComplete().BindUObject(this, &UCGRewardAPIClient::HandleGetRewardsResponse);
    Request->ProcessRequest();
}

void UCGRewardAPIClient::HandlePostRewardResponse(FHttpRequestPtr InRequest, FHttpResponsePtr InResponse, bool bWasSuccessful)
{
    bool bSuccess = false;
    FString ErrorMessage;

    if (!bWasSuccessful || !InResponse.IsValid())
    {
        ErrorMessage = TEXT("HTTP request failed or response invalid.");
    }
    else
    {
        const int32 StatusCode = InResponse->GetResponseCode();

        // 201 Created -> 정상 접수
        if (StatusCode == 201)
        {
            bSuccess = true;
        }
        else
        {
            // 409나 400 등일 수 있기 때문에 바디를 에러 메시지로 넘겨주기
            ErrorMessage = FString::Printf(TEXT("Unexpected status code: %d, body: %s"),
                StatusCode,
                *InResponse->GetContentAsString());
        }
    }

    if (OnPostRewardCompleted.IsBound())
    {
        OnPostRewardCompleted.Broadcast(bSuccess, ErrorMessage);
    }
}

void UCGRewardAPIClient::HandleGetRewardsResponse(FHttpRequestPtr InRequest, FHttpResponsePtr InResponse, bool bWasSuccessful)
{
    TArray<FCGRewardRecord> OutRewards;
    bool bSuccess = false;

    if (!bWasSuccessful || !InResponse.IsValid())
    {
        // 실패 -> 빈 배열 + false
        if (OnGetRewardsCompleted.IsBound())
        {
            OnGetRewardsCompleted.Broadcast(false, OutRewards);
        }
        return;
    }

    const int32 StatusCode = InResponse->GetResponseCode();
    if (StatusCode != 200)
    {
        if (OnGetRewardsCompleted.IsBound())
        {
            OnGetRewardsCompleted.Broadcast(false, OutRewards);
        }
        return;
    }

    const FString ResponseStr = InResponse->GetContentAsString();

    // JSON 파싱
    TSharedPtr<FJsonObject> RootObject;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseStr);

    if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
    {
        if (OnGetRewardsCompleted.IsBound())
        {
            OnGetRewardsCompleted.Broadcast(false, OutRewards);
        }
        return;
    }

    const TArray<TSharedPtr<FJsonValue>>* RewardsArrayPtr = nullptr;
    if (!RootObject->TryGetArrayField(TEXT("rewards"), RewardsArrayPtr) || !RewardsArrayPtr)
    {
        // rewards 필드가 없으면 그냥 빈 배열 반환
        if (OnGetRewardsCompleted.IsBound())
        {
            OnGetRewardsCompleted.Broadcast(true, OutRewards);
        }
        return;
    }

    for (const TSharedPtr<FJsonValue>& Value : *RewardsArrayPtr)
    {
        if (!Value.IsValid() || Value->Type != EJson::Object)
        {
            continue;
        }

        const TSharedPtr<FJsonObject> RewardObj = Value->AsObject();
        if (!RewardObj.IsValid())
        {
            continue;
        }

        FCGRewardRecord Record;

        // JSON 필드 -> 구조체로 복사 (없으면 빈 문자열)
        RewardObj->TryGetStringField(TEXT("jobId"), Record.JobId);
        RewardObj->TryGetStringField(TEXT("gameEventId"), Record.GameEventId);
        RewardObj->TryGetStringField(TEXT("itemCode"), Record.ItemCode);
        RewardObj->TryGetStringField(TEXT("status"), Record.Status);
        RewardObj->TryGetStringField(TEXT("tokenId"), Record.TokenId);
        RewardObj->TryGetStringField(TEXT("nftMetadataUrl"), Record.NftMetadataUrl);
        RewardObj->TryGetStringField(TEXT("txHash"), Record.TxHash);
        RewardObj->TryGetStringField(TEXT("error"), Record.Error);
        RewardObj->TryGetStringField(TEXT("updatedAt"), Record.UpdatedAt);

        OutRewards.Add(Record);
    }

    bSuccess = true;

    if (OnGetRewardsCompleted.IsBound())
    {
        OnGetRewardsCompleted.Broadcast(bSuccess, OutRewards);
    }
}
