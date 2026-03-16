// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "CGSkinListEntryWidget.generated.h"

/**
 * 
 */
UCLASS()
class CHRONICLEGATE_API UCGSkinListEntryWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
	
public:
    // ListView가 아이템을 배정할 때 호출
    virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UTextBlock> SkinNameText;

    // TODO: 스킨 아이콘 넣을지 말지
    //UPROPERTY(meta = (BindWidgetOptional))
    //UImage* SkinIcon;

    UPROPERTY()
    TObjectPtr<class UCGSkinListItemObject> ListItem;
	
	
};
