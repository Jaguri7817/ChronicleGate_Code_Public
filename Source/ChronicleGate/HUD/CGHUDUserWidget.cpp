// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/CGHUDUserWidget.h"
#include "CGHPBarWidget.h"
#include "Interface/CGCharacterHUDInterface.h"
#include "Character/CGStatComponent.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "TimerManager.h"

UCGHUDUserWidget::UCGHUDUserWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

void UCGHUDUserWidget::UpdateHPBar(float NewCurrentHP)
{
	
	HPBar->UpdateHPBar(NewCurrentHP);
}

void UCGHUDUserWidget::SetMaxHP(const float MaxHP)
{
	HPBar->SetMaxHP(MaxHP);
}

void UCGHUDUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	HPBar = Cast<UCGHPBarWidget>(GetWidgetFromName(TEXT("WidgetHPBar")));
	ensure(HPBar);

	ICGCharacterHUDInterface* HUDPawn = Cast<ICGCharacterHUDInterface>(GetOwningPlayerPawn());
	if (HUDPawn)
	{
		HUDPawn->SetUpHUDWidget(this);
	}

	if (RewardToastPanel)
	{
		RewardToastPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UCGHUDUserWidget::ShowBossRewardToast(FName ItemCode)
{
    UE_LOG(LogTemp, Warning, TEXT("[HUD] ShowBossRewardToast: %s"), *ItemCode.ToString());

    if (!RewardToastPanel || !RewardToastText)
    {
        UE_LOG(LogTemp, Error, TEXT("[HUD] RewardToast widgets not bound!"));
        return;
    }

    // "SK_CharM_Tusk" -> "Tusk"
    const FString RawCode = ItemCode.ToString();
    FString Left, Right;
    FString PrettyName = RawCode;

    if (RawCode.Split(TEXT("_"), &Left, &Right, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
    {
        PrettyName = Right;
    }

    // "{ItemName}을(를) 획득하였습니다." 형식으로 만들기
    const FText ItemNameText = FText::FromString(PrettyName);
    const FText ToastText = FText::Format(FText::FromString(TEXT("{0}을(를) 획득하였습니다.")), ItemNameText);

    RewardToastText->SetText(ToastText);
    RewardToastPanel->SetVisibility(ESlateVisibility::HitTestInvisible);

    // 타이머 리셋하고 2초 뒤 자동으로 숨기기
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(BossToastTimerHandle);
        World->GetTimerManager().SetTimer(BossToastTimerHandle, this, &UCGHUDUserWidget::HideBossRewardToast, 5.0f, false);
    }
}

void UCGHUDUserWidget::HideBossRewardToast()
{
    if (RewardToastPanel)
    {
        RewardToastPanel->SetVisibility(ESlateVisibility::Collapsed);
    }
}
