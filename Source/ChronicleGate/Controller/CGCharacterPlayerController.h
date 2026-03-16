// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CGCharacterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class CHRONICLEGATE_API ACGCharacterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ACGCharacterPlayerController();


protected:
	virtual void BeginPlay() override;

	
// HUD Section ===================================================================
protected:
	// 캐릭터 기본 HUD 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	TSubclassOf<class UCGHUDUserWidget> CGHUDUserWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD")
	TObjectPtr<class UCGHUDUserWidget> CGHUDUserWidget;


public:
	// 스킨 변경 UI 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UUserWidget> SkinChangeWidgetClass;

	UPROPERTY()
	TObjectPtr<class UUserWidget> SkinChangeWidget;

	// 스킨 변경 UI 열기 (서버 -> 클라)
	UFUNCTION(Client, Reliable)
	void ClientRPC_OpenSkinChangeUI();

	// 스킨 변경 UI 닫기 (클라에서 호출)
	UFUNCTION(BlueprintCallable, Category = "UI")
	void CloseSkinChangeUI();

	// NFT 획득 UI
	UFUNCTION(Client, Reliable)
	void ClientRPC_NotifyBossReward(FName ItemCode);

// ESC 메뉴 ===========================================================================
public:
	// ESC 토글 (BP/위젯에서 부를 수 있게)
	UFUNCTION(BlueprintCallable, Category = "ESC")
	void ToggleESCMenu();

	// 서버에 로비 귀환 요청
	UFUNCTION(Server, Reliable)
	void ServerRPC_RequestReturnToLobby();

protected:
	// ESC 메뉴 위젯 클래스 (에디터에서 WBP_EscMenu 지정)
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UCGESCMenuWidget> ESCMenuWidgetClass;

	// 실제 인스턴스
	UPROPERTY()
	TObjectPtr<class UCGESCMenuWidget> ESCMenuInstance;

// 지갑 주소 Section ===================================================================
public:
	UFUNCTION(Server, Reliable)
	void ServerRPC_SetWalletAddress(const FString& InWalletAddress);

	UFUNCTION(Client, Reliable)
	void ClientRPC_OnBossRewardGranted();
};
