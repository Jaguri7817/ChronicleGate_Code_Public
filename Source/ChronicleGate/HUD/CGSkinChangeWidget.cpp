// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/CGSkinChangeWidget.h"
#include "Components/Button.h"
#include "Components/ListView.h"
#include "Kismet/GameplayStatics.h"
#include "Controller/CGCharacterPlayerController.h"
#include "Equipment/CGCharacterSkinComponent.h"
#include "BlockChain/CGBackendSubsystem.h"
#include "Character/CGCharacterPlayer.h"
#include "Equipment/CGSkinListItemObject.h"
#include "Game/CGGameInstance.h"


void UCGSkinChangeWidget::NativeConstruct()
{
	Super::NativeConstruct();

    // 리스트뷰 선택 변경 델리게이트 (있다면)
    if (SkinListView)
    {
        // 처음 열릴 때 백엔드에서 스킨 리스트 가져와서 채우기
        if (UGameInstance* GI = GetWorld()->GetGameInstance())
        {
            if (auto* Backend = GI->GetSubsystem<UCGBackendSubsystem>())
            {
                // 1) 백엔드에서 스킨 리스트 갱신될 때마다 호출되도록 델리게이트 바인딩
                Backend->OnNFTSkinsUpdated.AddUObject(this, &UCGSkinChangeWidget::HandleNFTSkinsUpdated);
                // 2) 이미 캐시에 있는 스킨이 있다면 채워주기
                HandleNFTSkinsUpdated(Backend->GetOwnedNFTSkins());
            }
        }
        SkinListView->ClearSelection();
    }
}

void UCGSkinChangeWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // 버튼 델리게이트는 여기서 한 번만 바인딩
    if (ApplyButton)
    {
        ApplyButton->OnClicked.AddDynamic(this, &UCGSkinChangeWidget::OnApplyButtonClicked);
    }

    if (CloseButton)
    {
        CloseButton->OnClicked.AddDynamic(this, &UCGSkinChangeWidget::OnCloseButtonClicked);
    }

    if (SkinListView)
    {
        SkinListView->OnItemClicked().AddUObject(this, &UCGSkinChangeWidget::OnSkinItemClicked);
    }
}

void UCGSkinChangeWidget::OnSkinSelectionChanged(UObject* SelectedItem)
{
    if (UCGSkinListItemObject* Item = Cast<UCGSkinListItemObject>(SelectedItem))
    {
        SelectedSkin = Item->SkinInfo;  // <- 선택된 스킨 정보 저장
        bHasSelectedSkin = true;
    }
    else
    {
        bHasSelectedSkin = false;
    }
}

void UCGSkinChangeWidget::HandleNFTSkinsUpdated(const TArray<FCGNFTSkinInfo>& InSkins)
{
    UE_LOG(LogTemp, Log, TEXT("[SkinUI] HandleNFTSkinsUpdated: Count=%d"), InSkins.Num());

    if (!SkinListView)
    {
        return;
    }

    SkinListView->ClearListItems();

    for (const FCGNFTSkinInfo& Skin : InSkins)
    {
        UCGSkinListItemObject* ItemObj = NewObject<UCGSkinListItemObject>(this);
        ItemObj->Init(Skin);
        SkinListView->AddItem(ItemObj);
    }
    SkinListView->ClearSelection();
}

void UCGSkinChangeWidget::OnSkinItemClicked(UObject* InClickedItem)
{
    if (UCGSkinListItemObject* Item = Cast<UCGSkinListItemObject>(InClickedItem))
    {
        SelectedSkin = Item->SkinInfo;
        bHasSelectedSkin = true;

        UE_LOG(LogTemp, Log, TEXT("[SkinUI] Skin clicked: ItemCode=%s, TokenId=%s"), *SelectedSkin.ItemCode, *SelectedSkin.TokenId);
    }
    else
    {
        bHasSelectedSkin = false;
    }
}

void UCGSkinChangeWidget::OnApplyButtonClicked()
{
    UE_LOG(LogTemp, Log, TEXT("[SkinUI] Apply button clicked"));

    if (!bHasSelectedSkin)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SkinUI] No skin selected"));
        return;
    }

    // 플레이어 캐릭터 가져오기
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
    if (!PlayerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SkinUI] PlayerCharacter not found"));
        return;
    }

    // 캐릭터에 붙어있는 스킨 컴포넌트 찾기
    if (UCGCharacterSkinComponent* SkinComp = PlayerCharacter->FindComponentByClass<UCGCharacterSkinComponent>())
    {
        SkinComp->ApplySkinFromNFT(SelectedSkin);
        UE_LOG(LogTemp, Log, TEXT("[SkinUI] Applied skin from NFT: ItemCode=%s, TokenId=%s"), *SelectedSkin.ItemCode, *SelectedSkin.TokenId);
    }

    if (UCGGameInstance* GameInstance = GetWorld()->GetGameInstance<UCGGameInstance>())
    {
        GameInstance->SetSelectedSkin(SelectedSkin);
    }
}

void UCGSkinChangeWidget::OnCloseButtonClicked()
{
    // PlayerController의 CloseSkinChangeUI 호출
    if (auto* PlayerController = GetOwningPlayer<ACGCharacterPlayerController>())
    {
        PlayerController->CloseSkinChangeUI();
    }
    else
    {
        RemoveFromParent();
    }
}