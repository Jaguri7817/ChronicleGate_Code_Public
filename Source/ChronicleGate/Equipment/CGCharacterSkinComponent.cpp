// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/CGCharacterSkinComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/AssetManager.h"
#include "BlockChain/CGRewardType.h"

// Sets default values for this component's properties
UCGCharacterSkinComponent::UCGCharacterSkinComponent()
{
	
	PrimaryComponentTick.bCanEverTick = false;
       
    DefaultSkinId = TEXT("SK_CharM_Natural");
}


// Called when the game starts
void UCGCharacterSkinComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bApplyDefaultOnBeginPlay && !DefaultSkinId.IsEmpty())
	{
		ApplySkinById(DefaultSkinId);
	}
	
}

void UCGCharacterSkinComponent::ApplySkinById(const FString& InSkinId)
{
    if (InSkinId.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("[CharacterSkin] ApplySkinById called with empty SkinId"));
        return;
    }

    UAssetManager& AssetManager = UAssetManager::Get();
    const FPrimaryAssetId AssetId(TEXT("Skin"), FName(*InSkinId));

    // 먼저 스킨 데이터 에셋이 메모리에 올라와있는지 확인
    UObject* AssetObj = AssetManager.GetPrimaryAssetObject(AssetId);
    if (!AssetObj)
    {
        // 아직 메모리에 스킨 데이터 에셋이 없다면 로드 요청
        TSharedPtr<FStreamableHandle> Handle = AssetManager.LoadPrimaryAsset(AssetId);
        if (!Handle.IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("[CharacterSkin] Failed to create load handle for SkinId '%s'"), *InSkinId);
            return;
        }

        // 현재 로직은 동기식으로 쓰기 위해 로드 완료까지 블로킹 대기
        const EAsyncPackageState::Type Result = Handle->WaitUntilComplete(0.0f, false);
        if (Result != EAsyncPackageState::Complete)
        {
            UE_LOG(LogTemp, Warning, TEXT("[CharacterSkin] Skin load failed or timed out for '%s'"), *InSkinId);
            return;
        }

        // 로드가 끝나면 메모리에 올라온 스킨 데이터 에셋을 다시 가져옴
        AssetObj = AssetManager.GetPrimaryAssetObject(AssetId);
        if (!AssetObj)
        {
            UE_LOG(LogTemp, Warning, TEXT("[CharacterSkin] Loaded handle completed but asset object is null for '%s'"), *InSkinId);
            return;
        }
    }

    UCGSkinPrimaryData* FoundSkin = Cast<UCGSkinPrimaryData>(AssetObj);
    if (!FoundSkin)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CharacterSkin] No SkinData for '%s'"), *InSkinId);
        return;
    }

    // 메모리에 올라온 스킨 데이터 에셋의 SkinMesh는 Soft Object 참조이기 때문에 실제 스켈레탈 메쉬를 별도로 동기 로드
    USkeletalMesh* FoundMesh = FoundSkin->SkinMesh.LoadSynchronous();
    if (!FoundMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CharacterSkin] No SkeletalMesh mapped for SkinId '%s'"), *InSkinId);
        return;
    }

    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CharacterSkin] Owner is not a Character (Owner: %s)"), *GetNameSafe(GetOwner()));
        return;
    }

    USkeletalMeshComponent* MeshComponent = OwnerCharacter->GetMesh();
    if (!MeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CharacterSkin] Character has no Mesh component"));
        return;
    }

    MeshComponent->SetSkeletalMesh(FoundMesh);
    CurrentSkinId = InSkinId;

    UE_LOG(LogTemp, Log, TEXT("[CharacterSkin] Applied skin '%s'"), *InSkinId);
}

void UCGCharacterSkinComponent::ApplySkinFromNFT(const FCGNFTSkinInfo& InSkinInfo)
{
    UE_LOG(LogTemp, Log, TEXT("[SkinComp] ApplySkinFromNFT called: ItemCode=%s, TokenId=%s"), *InSkinInfo.ItemCode, *InSkinInfo.TokenId);
	ApplySkinById(InSkinInfo.ItemCode);
}

void UCGCharacterSkinComponent::ApplyDefaultSkin()
{
    if (!DefaultSkinId.IsEmpty())
    {
        ApplySkinById(DefaultSkinId);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[CharacterSkin] DefaultSkinId is empty, nothing to apply"));
    }
}


