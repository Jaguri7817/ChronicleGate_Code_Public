// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CGLoginPlayerController.generated.h"

/**
 * 
 */
class UUserWidget;

UCLASS()
class CHRONICLEGATE_API ACGLoginPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	ACGLoginPlayerController();
protected:
	// 컨트롤러가 월드에 참여할 때 (게임 시작 시) 호출됩니다.
	virtual void BeginPlay() override;
	
protected:
    // 에디터에서 띄울 '로그인 UI 위젯' 블루프린트를 지정할 변수
    // (예: WBP_Login)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Login")
    TSubclassOf<UUserWidget> LoginWidgetClass;

    // 레벨에 배치된 로그인 카메라를 찾기 위한 '태그'
    // (에디터에서 카메라 액터에 이 태그를 설정해야 함)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Login")
    FName LoginCameraTag = FName(TEXT("LoginCamera"));

private:
    // 생성된 로그인 위젯의 인스턴스를 저장할 변수
    UPROPERTY()
    TObjectPtr<UUserWidget> LoginWidgetInstance;

    TObjectPtr<class UCGLoginWidget> LoginWidget;
	
};
