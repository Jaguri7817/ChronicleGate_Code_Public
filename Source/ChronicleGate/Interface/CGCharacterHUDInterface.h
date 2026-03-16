// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "CGCharacterHUDInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCGCharacterHUDInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class CHRONICLEGATE_API ICGCharacterHUDInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void SetUpHUDWidget(class UCGHUDUserWidget* InHUDWidget) = 0;
	
};
