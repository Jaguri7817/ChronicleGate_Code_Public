// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CGLoginWidget.generated.h"

/**
 * 
 */
UCLASS()
class CHRONICLEGATE_API UCGLoginWidget : public UUserWidget
{
	GENERATED_BODY()
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UEditableTextBox> WalletAddressTextBox;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> LoginButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> ErrorText;
		
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnLoginClicked();

	UFUNCTION()
	void OnWalletCommitted(const FText& InText, ETextCommit::Type InCommitMethod);

	bool IsWalletAddressValid(const FString& Wallet) const;
};
