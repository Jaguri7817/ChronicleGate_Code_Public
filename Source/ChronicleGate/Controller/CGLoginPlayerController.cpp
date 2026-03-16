// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/CGLoginPlayerController.h"
#include "Camera/CameraActor.h"
#include "EngineUtils.h"
#include "HUD/CGLoginWidget.h"
#include "Blueprint/UserWidget.h"

ACGLoginPlayerController::ACGLoginPlayerController()
{
    static ConstructorHelpers::FClassFinder<UCGLoginWidget> WidgetClassRef(TEXT("/Game/HUD/WBP_Login.WBP_Login_C"));

    if (WidgetClassRef.Succeeded())
    {
        LoginWidgetClass = WidgetClassRef.Class;
    }
}

void ACGLoginPlayerController::BeginPlay()
{
    Super::BeginPlay();

    ACameraActor* LoginCamera = nullptr;

    // 태그로 찾기
    for (TActorIterator<ACameraActor> Iterator(GetWorld()); Iterator; ++Iterator)
    {
        if (Iterator->ActorHasTag(TEXT("LoginCamera")))
        {
            LoginCamera = *Iterator;
            break;
        }
    }

    if (LoginCamera)
    {
        SetViewTargetWithBlend(LoginCamera, 0.f); // 바로 전환
    }

    if (IsLocalController() && LoginWidgetClass)
    {
        LoginWidget = CreateWidget<UCGLoginWidget>(this, LoginWidgetClass);
        if (LoginWidget)
        {
            LoginWidget->AddToViewport();

            bShowMouseCursor = true;

            FInputModeUIOnly InputMode;
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            SetInputMode(InputMode);
        }
    }
    else    // 위젯 클래스가 안 들어있을 때 기본값 세팅
    {
        bShowMouseCursor = true;
        FInputModeUIOnly InputMode;
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        SetInputMode(InputMode);
    }
}