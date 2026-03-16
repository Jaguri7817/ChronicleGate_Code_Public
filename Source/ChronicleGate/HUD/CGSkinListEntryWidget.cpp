// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/CGSkinListEntryWidget.h"
#include "Equipment/CGSkinListItemObject.h"
#include "Components/TextBlock.h"

void UCGSkinListEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    ListItem = Cast<UCGSkinListItemObject>(ListItemObject);
    if (!ListItem) return;

    if (SkinNameText)
    {
        // 예: SkinId에서 "Base"만 추출해서 보여주기
        const FString& RawId = ListItem->SkinInfo.ItemCode; // "SK_CharM_Base"
        // 토큰 아이디 보여주기
        const FString& TokenId = ListItem->SkinInfo.TokenId;

        FString Suffix;
        if (RawId.Split(TEXT("_"), nullptr, &Suffix, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
        {
            // 예: "Base (Token ID: 3)"
            const FString Display = FString::Printf(TEXT("%s (Token ID: %s)"), *Suffix, *TokenId);
            SkinNameText->SetText(FText::FromString(Display));
        }
        else
        {
            const FString Display = FString::Printf(TEXT("%s (Token ID: %s)"), *RawId, *TokenId);
            SkinNameText->SetText(FText::FromString(Display));
        }
    }
}


