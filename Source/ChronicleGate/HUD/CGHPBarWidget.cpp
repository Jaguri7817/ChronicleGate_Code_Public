// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/CGHPBarWidget.h"
#include "Components/ProgressBar.h"
#include "Interface/CGCharacterWidgetInterface.h"

UCGHPBarWidget::UCGHPBarWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	MaxHP = -1.0f;
}

void UCGHPBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	HPProgressBar = Cast<UProgressBar>(GetWidgetFromName(TEXT("HPBar"))); // 여기에 내가 지정한 progressbar 이름을 넣기
	ensure(HPProgressBar);

	ICGCharacterWidgetInterface* CharacterWidget = Cast<ICGCharacterWidgetInterface>(OwningActor);
	if (CharacterWidget)
	{
		CharacterWidget->SetUpCharacterWidget(this);
	}

}

void UCGHPBarWidget::UpdateHPBar(float NewCurrentHP)
{
	ensure(MaxHP > 0.0f);
	if (HPProgressBar)
	{
		HPProgressBar->SetPercent(NewCurrentHP / MaxHP);
	}
}
