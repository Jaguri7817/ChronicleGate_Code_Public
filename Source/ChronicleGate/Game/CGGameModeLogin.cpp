// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/CGGameModeLogin.h"
#include "Controller/CGLoginPlayerController.h"

ACGGameModeLogin::ACGGameModeLogin()
{

	DefaultPawnClass = nullptr;
	PlayerControllerClass = ACGLoginPlayerController::StaticClass();
}
