// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "CGInteractInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCGInteractInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class CHRONICLEGATE_API ICGInteractInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// UI 프롬프트용(선택)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable) FText GetInteractPrompt() const;

	// 상호작용 가능 여부(거리/상태)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable) bool CanInteract(APawn* InstigatorPawn) const;

	// 실제 실행(서버에서 실행됨)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable) void Interact(APawn* InstigatorPawn);
	
};
