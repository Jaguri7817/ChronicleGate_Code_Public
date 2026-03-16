// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/CGLoginWidget.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Game/CGGameInstance.h"
#include "BlockChain/CGBackendSubsystem.h"

void UCGLoginWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (LoginButton)
	{
		LoginButton->OnClicked.AddDynamic(this, &UCGLoginWidget::OnLoginClicked);
	}

	if (WalletAddressTextBox)
	{
		WalletAddressTextBox->OnTextCommitted.AddDynamic(this, &UCGLoginWidget::OnWalletCommitted);
	}
}

void UCGLoginWidget::OnLoginClicked()
{
	FString Wallet = WalletAddressTextBox->GetText().ToString();
	Wallet.TrimStartAndEndInline();	// 앞뒤 공백 제거

	if (!IsWalletAddressValid(Wallet))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid wallet address: %s"), *Wallet);
		// 에러 텍스트 위젯
		if (ErrorText)
		{
			ErrorText->SetText(FText::FromString(TEXT("지갑 주소 형식이 올바르지 않습니다.")));
			ErrorText->SetVisibility(ESlateVisibility::Visible);
		}
		return;
	}

	if (ErrorText)
	{
		ErrorText->SetText(FText::GetEmpty());
		ErrorText->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (UCGGameInstance* GameInstance = GetWorld()->GetGameInstance<UCGGameInstance>())
	{
		GameInstance->SetWalletAddress(Wallet);

		if (UCGBackendSubsystem* Backend =
			GameInstance->GetSubsystem<UCGBackendSubsystem>())
		{
			Backend->SetWalletAddress(Wallet);
			Backend->RefreshRewards();   // 첫 로그인 시 보상 목록 가져오기
		}
	
		if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
		{
			const FString& ServerAddress = GameInstance->GetServerAddress();
			PlayerController->ClientTravel(ServerAddress, ETravelType::TRAVEL_Absolute);
		}
	}	
}

void UCGLoginWidget::OnWalletCommitted(const FText& InText, ETextCommit::Type InCommitMethod)
{
	if (InCommitMethod == ETextCommit::OnEnter)
	{
		OnLoginClicked();	// 엔터 시 버튼 클릭한 것처럼 처리
	}
}

bool UCGLoginWidget::IsWalletAddressValid(const FString& InWallet) const
{
	if (InWallet.IsEmpty())
	{
		if (ErrorText)
		{
			ErrorText->SetText(FText::FromString(TEXT("지갑 주소를 입력해주세요.")));
			ErrorText->SetVisibility(ESlateVisibility::Visible);
		}
	}
	if (InWallet.Len() != 42)
	{
		return false;
	}
	if (!InWallet.StartsWith(TEXT("0x")) && !InWallet.StartsWith(TEXT("0X")))
	{
		return false;
	}

	for (int32 i = 2; i < InWallet.Len(); ++i)
	{
		const TCHAR C = InWallet[i];

		const bool bIsDigit = (C >= '0' && C <= '9');
		const bool bIsLowerHex = (C >= 'a' && C <= 'f');
		const bool bIsUpperHex = (C >= 'A' && C <= 'F');

		if (!bIsDigit && !bIsLowerHex && !bIsUpperHex)
		{
			return false;
		}
	}

	return true;
}
