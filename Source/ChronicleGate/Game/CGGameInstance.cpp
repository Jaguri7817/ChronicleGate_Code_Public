// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/CGGameInstance.h"

UCGGameInstance::UCGGameInstance()
{
}

void UCGGameInstance::Init()
{
	Super::Init();
	UserWalletAddress = TEXT("");
}

void UCGGameInstance::Shutdown()
{
	Super::Shutdown();
}

void UCGGameInstance::SetSelectedSkin(const FCGNFTSkinInfo& InSkin)
{
    SelectedSkin = InSkin;
    bHasSelectedSkin = true;
}


bool UCGGameInstance::GetSelectedSkin(FCGNFTSkinInfo& OutSkin) const
{
    if (!bHasSelectedSkin)
    {
        return false;
    }
    OutSkin = SelectedSkin;
    return true;
}