// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CGESCMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class CHRONICLEGATE_API UCGESCMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

    // BP에서 버튼 OnClicked에 바로 물릴 함수들
    UFUNCTION(BlueprintCallable, Category = "ESC")
    void OnClick_ReturnToLobby();

    UFUNCTION(BlueprintCallable, Category = "ESC")
    void OnClick_QuitGame();

    UFUNCTION(BlueprintCallable, Category = "ESC")
    void OnClick_Resume(); // 메뉴 닫기 (게임 계속하기)
	
	
};
