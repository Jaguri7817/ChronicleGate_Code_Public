// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BlockChain/CGRewardType.h"                // FCGNFTSkinInfo 쓸 거면
#include "Equipment/CGSkinPrimaryData.h"
#include "CGCharacterSkinComponent.generated.h"

/**
 * 캐릭터의 스킨(USkeletalMesh)을 SkinId로 교체하는 컴포넌트.
 * - 캐릭터( CGCharacterPlayer )에 붙여서 사용.
 */

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CHRONICLEGATE_API UCGCharacterSkinComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCGCharacterSkinComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

    // SkinId로 스킨 적용 (예: "SK_CharM_Base", "SK_CharM_Bladed" ...)
    UFUNCTION(BlueprintCallable, Category = "Character Skin")
    void ApplySkinById(const FString& SkinId);

public:
    // NFT 스킨 정보 구조체에서 바로 적용하고 싶을 때 사용
    UFUNCTION(BlueprintCallable, Category = "Character Skin")
    void ApplySkinFromNFT(const FCGNFTSkinInfo& SkinInfo);

protected:
    // DefaultSkinId 로 돌아가기
    UFUNCTION(BlueprintCallable, Category = "Character Skin")
    void ApplyDefaultSkin();

    // 현재 적용된 스킨 Id
    UFUNCTION(BlueprintPure, Category = "Character Skin")
    const FString& GetCurrentSkinId() const { return CurrentSkinId; }

protected:
    // 초기에 적용할 스킨 Id (옵션)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Skin")
    FString DefaultSkinId;

    // BeginPlay 때 DefaultSkinId를 자동으로 적용할지 여부
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Skin")
    bool bApplyDefaultOnBeginPlay = true;

    // 현재 적용된 스킨 Id (디버그/표시용)
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character Skin")
    FString CurrentSkinId;
	
};
