#pragma once

#include "CoreMinimal.h"
#include "CGRewardType.generated.h"

USTRUCT(BlueprintType)
struct FCGRewardRecord
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString JobId;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString GameEventId;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString ItemCode;      // 예: "SK_CharM_Forge"

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString Status;   // "COMPLETED", "PENDING", "FAILED"

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString TokenId;

    UPROPERTY(BlueprintReadOnly)
    FString NftMetadataUrl;  // 토큰 메타데이터

    UPROPERTY(BlueprintReadOnly)
    FString TxHash;

    UPROPERTY(BlueprintReadOnly)
    FString Error;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString UpdatedAt;

};

USTRUCT(BlueprintType)
struct FCGNFTSkinInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, EditAnywhere)
    FString TokenId;         // 온체인 NFT 토큰 ID
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString ItemCode;        // SK_CharM_Forge 같은 코드

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString DisplayName;    // UI용 이름

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString MetadataUrl;    // 토큰 메타데이터
};