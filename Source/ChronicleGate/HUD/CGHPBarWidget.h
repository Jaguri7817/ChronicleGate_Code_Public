// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CGUserWidget.h"
#include "CGHPBarWidget.generated.h"

/**
 * 
 */
UCLASS()
class CHRONICLEGATE_API UCGHPBarWidget : public UCGUserWidget
{
	GENERATED_BODY()
	
public:
	UCGHPBarWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;

public:
	FORCEINLINE void SetMaxHP(float NewMaxHP) { MaxHP = NewMaxHP; }
	void UpdateHPBar(float NewCurrentHP);

protected:
	UPROPERTY()
	TObjectPtr<class UProgressBar> HPProgressBar;

	UPROPERTY()
	float MaxHP;
	
	
};
