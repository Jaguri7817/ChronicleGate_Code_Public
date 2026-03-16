#pragma once

#include "CoreMinimal.h"
#include "CGTokenType.generated.h"

USTRUCT(BlueprintType)
struct FTokenMintResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    bool bSuccess = false;

    UPROPERTY(BlueprintReadOnly)
    FString TxHash;

    UPROPERTY(BlueprintReadOnly)
    FString Amount;

    UPROPERTY(BlueprintReadOnly)
    FString Recipient;

    UPROPERTY(BlueprintReadOnly)
    FString ExplorerUrl;

    UPROPERTY(BlueprintReadOnly)
    FString ErrorCode;

    UPROPERTY(BlueprintReadOnly)
    FString ErrorMessage;
};

USTRUCT(BlueprintType)
struct FTokenBurnResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    bool bSuccess = false;

    UPROPERTY(BlueprintReadOnly)
    FString TxHash;

    UPROPERTY(BlueprintReadOnly)
    FString BurnedAmount;

    UPROPERTY(BlueprintReadOnly)
    FString RemainingAllowance;

    UPROPERTY(BlueprintReadOnly)
    FString ExplorerUrl;

    UPROPERTY(BlueprintReadOnly)
    FString ErrorCode;

    UPROPERTY(BlueprintReadOnly)
    FString ErrorMessage;
};

USTRUCT(BlueprintType)
struct FTokenBalanceResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    bool bSuccess = false;

    UPROPERTY(BlueprintReadOnly)
    FString Address;

    UPROPERTY(BlueprintReadOnly)
    FString Balance;

    UPROPERTY(BlueprintReadOnly)
    FString Symbol;

    UPROPERTY(BlueprintReadOnly)
    FString ErrorCode;

    UPROPERTY(BlueprintReadOnly)
    FString ErrorMessage;
};