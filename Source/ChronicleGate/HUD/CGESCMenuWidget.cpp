// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/CGESCMenuWidget.h"
#include "Controller/CGCharacterPlayerController.h"
#include "Kismet/KismetSystemLibrary.h"

void UCGESCMenuWidget::OnClick_ReturnToLobby()
{
    if (ACGCharacterPlayerController* CGPlayerController = Cast<ACGCharacterPlayerController>(GetOwningPlayer()))
    {
        CGPlayerController->ServerRPC_RequestReturnToLobby();
    }
}

void UCGESCMenuWidget::OnClick_QuitGame()
{
    if (ACGCharacterPlayerController* CGPlayerController = Cast<ACGCharacterPlayerController>(GetOwningPlayer()))
    {
        UKismetSystemLibrary::QuitGame(this, CGPlayerController, EQuitPreference::Quit, false);
    }
}

void UCGESCMenuWidget::OnClick_Resume()
{
    if (ACGCharacterPlayerController* CGPlayerController = Cast<ACGCharacterPlayerController>(GetOwningPlayer()))
    {
        CGPlayerController->ToggleESCMenu();
    }
}
