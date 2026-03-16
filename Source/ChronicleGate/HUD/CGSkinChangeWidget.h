// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BlockChain/CGRewardType.h"
#include "CGSkinChangeWidget.generated.h"

/**
 * 
 */
UCLASS()
class CHRONICLEGATE_API UCGSkinChangeWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    virtual void NativeConstruct() override;
    virtual void NativeOnInitialized() override;

    // 외부에서 현재 선택된 스킨을 세팅해줄 때도 쓸 수 있음
    void SetSelectedSkin(const FCGNFTSkinInfo& InSkin) { SelectedSkin = InSkin; bHasSelectedSkin = true; }

protected:
    // BP 디자이너에서 배치한 위젯들과 연결
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UButton> ApplyButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UButton> CloseButton;

    // 예: 스킨 리스트를 보여줄 ListView (있으면)
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<class UListView> SkinListView;

    // C++에서 사용할 선택된 스킨 정보
    FCGNFTSkinInfo SelectedSkin;
    bool bHasSelectedSkin = false;

    // 버튼 클릭시 호출될 C++ 함수들
    UFUNCTION()
    void OnApplyButtonClicked();

    UFUNCTION()
    void OnCloseButtonClicked();

    // 스킨 리스트가 있다면, 선택 변경 시 호출
    UFUNCTION()
    void OnSkinSelectionChanged(UObject* SelectedItem);
	
    // 스킨 목록이 갱신되었을 때 호출될 핸들러
    void HandleNFTSkinsUpdated(const TArray<FCGNFTSkinInfo>& InSkins);

    UFUNCTION()
    void OnSkinItemClicked(UObject* InClickedItem);
};
