// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/CGCharacterPlayerController.h"
#include "HUD/CGHUDUserWidget.h"
#include "HUD/CGSkinChangeWidget.h"
#include "Game/CGGameInstance.h"
#include "BlockChain/CGBackendSubsystem.h"
#include "HUD/CGESCMenuWidget.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Game/CGStageStateBase.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

ACGCharacterPlayerController::ACGCharacterPlayerController()
{
	static ConstructorHelpers::FClassFinder<UCGHUDUserWidget> CGHUDUserWidgetRef(TEXT("/Game/HUD/WBP_CGHUD.WBP_CGHUD_C"));
	if (CGHUDUserWidgetRef.Class)
	{
		CGHUDUserWidgetClass = CGHUDUserWidgetRef.Class;
	}

	static ConstructorHelpers::FClassFinder<UCGSkinChangeWidget> CGSkinChangeWidgetRef(TEXT("/Game/HUD/WBP_SkinChange.WBP_SkinChange_C"));
	if (CGSkinChangeWidgetRef.Class)
	{
		SkinChangeWidgetClass = CGSkinChangeWidgetRef.Class;
	}

	static ConstructorHelpers::FClassFinder<UCGESCMenuWidget> ESCMenuWidgetRef(TEXT("/Game/HUD/WBP_ESCMenu.WBP_ESCMenu_C"));
	if (ESCMenuWidgetRef.Class)
	{
		ESCMenuWidgetClass = ESCMenuWidgetRef.Class;
	}

	bShowMouseCursor = false;
	ESCMenuInstance = nullptr;
}

void ACGCharacterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameOnly GameOnlyInputMode;
	SetInputMode(GameOnlyInputMode);

	if (IsLocalController())
	{
		if (!CGHUDUserWidget && CGHUDUserWidgetClass)
		{
			CGHUDUserWidget = CreateWidget<UCGHUDUserWidget>(this, CGHUDUserWidgetClass);
			if (CGHUDUserWidget)
			{
				CGHUDUserWidget->AddToViewport();
			}
		}

		if (UCGGameInstance* GameInstance = GetWorld()->GetGameInstance<UCGGameInstance>())
		{
			const FString WalletAddress = GameInstance->GetWalletAddress();
			ServerRPC_SetWalletAddress(WalletAddress);
		}
	}
}

void ACGCharacterPlayerController::ClientRPC_OpenSkinChangeUI_Implementation()
{
	if (!SkinChangeWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PC] SkinChangeWidgetClass is not set"));
		return;
	}

	if (UCGGameInstance* GameInstance = GetWorld()->GetGameInstance<UCGGameInstance>())
	{
		if (UCGBackendSubsystem* Backend = GameInstance->GetSubsystem<UCGBackendSubsystem>())
		{
			Backend->RefreshRewards();
		}
	}

	if (!SkinChangeWidget)
	{
		SkinChangeWidget = CreateWidget<UUserWidget>(this, SkinChangeWidgetClass);
	}

	if (!SkinChangeWidget) return;

	SkinChangeWidget->AddToViewport();

	// 입력 모드를 UI 쪽으로
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(SkinChangeWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	SetInputMode(InputMode);
	bShowMouseCursor = true;

	UE_LOG(LogTemp, Log, TEXT("[PC] Opened SkinChange UI"));
}

void ACGCharacterPlayerController::CloseSkinChangeUI()
{
	if (SkinChangeWidget)
	{
		SkinChangeWidget->RemoveFromParent();
	}

	// 다시 게임 입력 모드로
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;

	UE_LOG(LogTemp, Log, TEXT("[PC] Closed SkinChange UI"));
}

void ACGCharacterPlayerController::ClientRPC_NotifyBossReward_Implementation(FName ItemCode)
{
	UE_LOG(LogTemp, Warning, TEXT("[NotifyBossReward] HUD=%s"),	*GetNameSafe(CGHUDUserWidget));
	// 1) "SK_CharM_Tusk" -> FString으로 변환
	const FString RawCode = ItemCode.ToString();

	// 2) 마지막 '_' 기준으로 잘라서 오른쪽 토큰(Tusk)만 사용
	FString Left, Right;
	FString PrettyName = RawCode;   // 기본값 (언더바 없으면 그냥 전체)

	if (RawCode.Split(TEXT("_"), &Left, &Right, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
	{
		PrettyName = Right; // 3) 여기서 "Tusk"만 남음
	}

	if (CGHUDUserWidget)
	{
		CGHUDUserWidget->ShowBossRewardToast(FName(*PrettyName));
		UE_LOG(LogTemp, Warning, TEXT("[NotifyBossReward] Call HUD Toast: %s"), *PrettyName);
	}

#if WITH_EDITOR
	if (GEngine)
	{
		const FString Msg = FString::Printf(TEXT("%s 을(를) 획득하였습니다."), *PrettyName);
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, Msg);
	}
#endif
}

void ACGCharacterPlayerController::ToggleESCMenu()
{
	if (!ESCMenuWidgetClass) return;

	UE_LOG(LogTemp, Warning, TEXT("[PlayerController] Toggle ESC Menu"));
	if (ESCMenuInstance && ESCMenuInstance->IsInViewport())
	{
		// 닫기
		ESCMenuInstance->RemoveFromParent();
		ESCMenuInstance = nullptr;

		bShowMouseCursor = false;

		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
	}
	else
	{
		// 열기
		ESCMenuInstance = CreateWidget<UCGESCMenuWidget>(this, ESCMenuWidgetClass);
		if (ESCMenuInstance)
		{
			ESCMenuInstance->AddToViewport();

			bShowMouseCursor = true;

			FInputModeGameAndUI InputMode;
			InputMode.SetHideCursorDuringCapture(false);
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);
		}
	}
}

void ACGCharacterPlayerController::ServerRPC_RequestReturnToLobby_Implementation()
{
	ACGStageStateBase* StageState = GetWorld()->GetGameState<ACGStageStateBase>();
	if (StageState)
	{
		StageState->DoTravelToLobby();
	}
}

void ACGCharacterPlayerController::ClientRPC_OnBossRewardGranted_Implementation()
{
	if (UCGGameInstance* GameInstance = GetWorld()->GetGameInstance<UCGGameInstance>())
	{
		if (UCGBackendSubsystem* Backend = GameInstance->GetSubsystem<UCGBackendSubsystem>())
		{
			UE_LOG(LogTemp, Log, TEXT("[Client] Boss reward granted! Refreshing rewards..."));
			Backend->RefreshRewards();
		}
	}
}

void ACGCharacterPlayerController::ServerRPC_SetWalletAddress_Implementation(const FString& InWalletAddress)
{
	UE_LOG(LogTemp, Log, TEXT("[Server] Server_SetWalletAddress: %s"), *InWalletAddress);

	// 서버쪽 BackendSubsystem에도 세팅
	if (UCGGameInstance* GameInstance = GetWorld()->GetGameInstance<UCGGameInstance>())
	{
		if (UCGBackendSubsystem* Backend = GameInstance->GetSubsystem<UCGBackendSubsystem>())
		{
			Backend->SetWalletAddress(InWalletAddress);
			UE_LOG(LogTemp, Log, TEXT("[Server] Backend Wallet set = %s"), *InWalletAddress);
		}
	}
}
