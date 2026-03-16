// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Character/CGStatData.h"
#include "CGHUDUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class CHRONICLEGATE_API UCGHUDUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UCGHUDUserWidget(const FObjectInitializer& ObjectInitializer);
	void UpdateHPBar(float NewCurrentHP);
	void SetMaxHP(const float MaxHP);

protected:
	virtual void NativeConstruct();

protected:
	UPROPERTY()
	TObjectPtr<class UCGHPBarWidget> HPBar;

public:
	UFUNCTION()
	void ShowBossRewardToast(FName ItemCode);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> RewardToastText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UWidget> RewardToastPanel;

	FTimerHandle BossToastTimerHandle;

	UFUNCTION()
	void HideBossRewardToast();
};
