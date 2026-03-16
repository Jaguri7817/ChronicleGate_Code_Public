// Fill out your copyright notice in the Description page of Project Settings.


#include "BlockChain/CGBackendSubsystem.h"
#include "CGRewardAPIClient.h"
#include "CGTokenAPIClient.h"
#include "Engine/Engine.h"
#include "ApiConfig/ApiConfig.h"
#include "Controller/CGCharacterPlayerController.h"

void UCGBackendSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

    // NFT Skin Part
	RewardAPIClient = NewObject<UCGRewardAPIClient>(this);
	if (RewardAPIClient)
	{
		RewardAPIClient->Init(ApiConfig::GetRewardApiUrl());	// TEXT 안에 백엔드 url 링크(ApiConfig 파일로 따로 관리)

		// 델리게이트 바인딩
		RewardAPIClient->OnPostRewardCompleted.AddUObject(this, &UCGBackendSubsystem::HandlePostRewardCompleted);
		RewardAPIClient->OnGetRewardsCompleted.AddUObject(this, &UCGBackendSubsystem::HandleGetRewardsCompleted);
	}
    
    // Token Part
    TokenAPIClient = NewObject<UCGTokenAPIClient>(this);
    if (TokenAPIClient)
    {
        TokenAPIClient->Init(ApiConfig::GetTokenApiUrl());      // TEXT 안에 백엔드 url 링크(ApiConfig 파일로 따로 관리)

        // 델리게이트 바인딩
        TokenAPIClient->OnMintCompleted.AddDynamic(this, &UCGBackendSubsystem::HandleMintResult);
        TokenAPIClient->OnBurnCompleted.AddDynamic(this, &UCGBackendSubsystem::HandleBurnResult);
        TokenAPIClient->OnBalanceCompleted.AddDynamic(this, &UCGBackendSubsystem::HandleBalanceResult);
    }
}

void UCGBackendSubsystem::Deinitialize()
{
	if (RewardAPIClient)
	{
		RewardAPIClient->OnPostRewardCompleted.Clear();
		RewardAPIClient->OnGetRewardsCompleted.Clear();
		RewardAPIClient = nullptr;
	}

    if (TokenAPIClient)
    {
        TokenAPIClient->OnMintCompleted.Clear();
        TokenAPIClient->OnBurnCompleted.Clear();
        TokenAPIClient->OnBalanceCompleted.Clear();
        TokenAPIClient = nullptr;
    }

	Super::Deinitialize();
}

void UCGBackendSubsystem::RequestDungeonClearReward(const FString& InDungeonId, const FString& InItemCode)
{
    if (!RewardAPIClient)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BackendSubsystem] RewardAPIClient is null."));
        return;
    }

    if (WalletAddress.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("[BackendSubsystem] WalletAddress is empty. SetWalletAddress() 먼저 호출 필요."));
        return;
    }

    if (InDungeonId.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("[BackendSubsystem] DungeonId is empty."));
        return;
    }

    if (InItemCode.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("[BackendSubsystem] ItemCode is empty."));
        return;
    }

    const FDateTime NowUtc = FDateTime::UtcNow();
    const FString Timestamp = NowUtc.ToString(TEXT("%Y%m%d_%H%M%S"));

    // gameEventId 형식: dungeon_clear_<던전ID>_<타임스탬프>
    const FString GameEventId = FString::Printf(TEXT("dungeon_clear_%s_%s"), *InDungeonId, *Timestamp);

    UE_LOG(LogTemp, Log, TEXT("[BackendSubsystem] RequestDungeonClearReward: Wallet=%s, GameEventId=%s, ItemCode=%s"), *WalletAddress, *GameEventId, *InItemCode);

    RewardAPIClient->PostRewardRequest(WalletAddress, GameEventId, InItemCode);
}

void UCGBackendSubsystem::RefreshRewards()
{
    if (!RewardAPIClient)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BackendSubsystem] RewardAPIClient is null."));
        return;
    }

    if (WalletAddress.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("[BackendSubsystem] WalletAddress is empty. SetWalletAddress() 먼저 호출 필요."));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[BackendSubsystem] RefreshRewards: Wallet=%s"), *WalletAddress);

    RewardAPIClient->GetRewards(WalletAddress);
}

void UCGBackendSubsystem::HandlePostRewardCompleted(bool bSuccess, const FString& ErrorMessage)
{
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("[BackendSubsystem] PostRewardRequest 성공. 최신 보상 목록 다시 가져오기."));

        // 보상 요청이 성공했으면, 곧 COMPLETED/PENDING 상태가 바뀔 수 있으니
        // 바로 보상 목록을 갱신해줘도 됨 (원하면 생략 가능)
        RefreshRewards();
        if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
        {
            if (ACGCharacterPlayerController* CGPlayerController = Cast<ACGCharacterPlayerController>(PlayerController))
            {
                CGPlayerController->ClientRPC_OnBossRewardGranted();
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[BackendSubsystem] PostRewardRequest 실패: %s"), *ErrorMessage);
    }
}

void UCGBackendSubsystem::HandleGetRewardsCompleted(bool bSuccess, const TArray<FCGRewardRecord>& Rewards)
{
    if (!bSuccess)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BackendSubsystem] GetRewards 실패. 캐시/스킨 리스트 초기화."));

        CachedRewards = Rewards;  // 보통 빈 배열일 것
        if (OnRewardsUpdated.IsBound())
        {
            OnRewardsUpdated.Broadcast(CachedRewards);
        }

        OwnedNFTSkins.Empty();
        if (OnNFTSkinsUpdated.IsBound())
        {
            OnNFTSkinsUpdated.Broadcast(OwnedNFTSkins);
        }
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[BackendSubsystem] GetRewards 성공. Count=%d"), Rewards.Num());

    // 1) 보상 전체를 캐시에 저장
    CachedRewards = Rewards;

    // 2) Raw 보상 리스트 브로드캐스트 (원하면 UI에서 PENDING/FAILED도 그리드에 보여줄 수 있음)
    if (OnRewardsUpdated.IsBound())
    {
        OnRewardsUpdated.Broadcast(CachedRewards);
    }

    // 3) COMPLETED + nft != null 만 골라서 OwnedNFTSkins 재구성
    RebuildOwnedNFTSkinsFromRewards();
}

void UCGBackendSubsystem::RebuildOwnedNFTSkinsFromRewards()
{
    OwnedNFTSkins.Empty();

    for (const FCGRewardRecord& Record : CachedRewards)
    {
        // status == "FAILED"만 제외하고 나머지(PENDING, PROCESSING, COMPLETED) 모두 사용
        if (Record.Status.Equals(TEXT("FAILED"), ESearchCase::IgnoreCase))
        {
            UE_LOG(LogTemp, Warning, TEXT("[BackendSubsystem] Skip FAILED reward: %s"), *Record.GameEventId);
            continue;
        }

        // nft 필드가 비어있으면 스킵
        if (Record.ItemCode.IsEmpty())
        {
            continue;
        }

        FCGNFTSkinInfo SkinInfo;
        
        SkinInfo.ItemCode = Record.ItemCode;    // 어떤 스킨인지
        SkinInfo.TokenId = Record.TokenId;      // 온체인 토큰 ID

        // MVP에서는 NFT 문자열 자체를 SkinId/DisplayName으로 사용
        // 나중에 DataTable이나 Map으로 "SK_CharM_Forge" -> 실제 에셋 Path/이름 변환
        SkinInfo.MetadataUrl = Record.NftMetadataUrl;
        SkinInfo.DisplayName = Record.ItemCode;

        OwnedNFTSkins.Add(SkinInfo);
    }

    UE_LOG(LogTemp, Log, TEXT("[BackendSubsystem] OwnedNFTSkins rebuilt. Count=%d"), OwnedNFTSkins.Num());

    if (OnNFTSkinsUpdated.IsBound())
    {
        OnNFTSkinsUpdated.Broadcast(OwnedNFTSkins);
    }
}

void UCGBackendSubsystem::RequestMintToken(const FString& ToAddress, const FString& Amount)
{
    if (!TokenAPIClient)
    {
        UE_LOG(LogTemp, Error, TEXT("[Backend] TokenAPIClient is null"));
        return;
    }

    // 여기서 UI에 로딩 켜도 됨 (예: bIsWaitingTokenTx = true)
    TokenAPIClient->MintToken(ToAddress, Amount);
}

void UCGBackendSubsystem::RequestBurnToken(const FString& UserAddress, const FString& Amount)
{
    if (!TokenAPIClient)
    {
        UE_LOG(LogTemp, Error, TEXT("[Backend] TokenAPIClient is null"));
        return;
    }

    TokenAPIClient->BurnToken(UserAddress, Amount);
}

void UCGBackendSubsystem::RequestTokenBalance(const FString& Address)
{
    if (!TokenAPIClient)
    {
        UE_LOG(LogTemp, Error, TEXT("[Backend] TokenAPIClient is null"));
        return;
    }

    TokenAPIClient->GetBalance(Address);
}

void UCGBackendSubsystem::HandleMintResult(const FTokenMintResult& Result)
{
    if (Result.bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("[Backend] Mint OK - TxHash: %s, Amount: %s, To: %s"), *Result.TxHash, *Result.Amount, *Result.Recipient);

        // TODO: 여기서 UI에 "발행 성공" 토스트 띄우거나, 게임 내 토큰 잔액 재조회 트리거
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[Backend] Mint Failed - %s (%s)"), *Result.ErrorMessage, *Result.ErrorCode);
        // TODO: UI에 에러 메시지 표시
    }
}

void UCGBackendSubsystem::HandleBurnResult(const FTokenBurnResult& Result)
{
    if (Result.bSuccess)
    {
        // 예: 아이템 구매 완료 처리, 잔액 갱신 요청 등
        UE_LOG(LogTemp, Log, TEXT("[Backend] Burn OK - TxHash: %s, Burned: %s, RemainAllowance: %s"), *Result.TxHash, *Result.BurnedAmount, *Result.RemainingAllowance);
        
    }
    else
    {
        // code == INSUFFICIENT_ALLOWANCE 면 "먼저 approve 하라" 메시지
        UE_LOG(LogTemp, Error, TEXT("[Backend] Burn Failed - %s (%s)"), *Result.ErrorMessage, *Result.ErrorCode);

    }
}

void UCGBackendSubsystem::HandleBalanceResult(const FTokenBalanceResult& Result)
{
    if (Result.bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("[Backend] Balance - %s: %s %s"), *Result.Address, *Result.Balance, *Result.Symbol);

        // 여기서 GameInstance나 State에 "현재 토큰 잔액" 저장하고, HUD 위젯 갱신 트리거 하는 식으로 쓰기
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[Backend] GetBalance Failed - %s (%s)"), *Result.ErrorMessage, *Result.ErrorCode);
    }
}
