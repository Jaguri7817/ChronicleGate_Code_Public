// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/CGCharacterBase.h"
#include "InputActionValue.h"
#include "Net/UnrealNetwork.h"
#include "Interface/CGAnimationAttackInterface.h"
#include "Interface/CGInteractInterface.h"
#include "Interface/CGCharacterHUDInterface.h"
#include "CGCharacterPlayer.generated.h"

/**
 * 
 */
UCLASS()
class CHRONICLEGATE_API ACGCharacterPlayer : public ACGCharacterBase, public ICGAnimationAttackInterface, public ICGCharacterHUDInterface
{
	GENERATED_BODY()

public:
	ACGCharacterPlayer();

protected:
	// 입력 매핑 컨텍스트를 할당하는 역할
	virtual void BeginPlay() override;

public:
	// 언리얼엔진의 Input System에서 입력 액션과 Move, Look 함수를 매핑시켜주는 역할
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

// ================================================================================================================================================
// 카메라 Section 
protected:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> FollowCamera;

// ================================================================================================================================================
// 입력 Section
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> RollAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> ESCMenuAction;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Input_ToggleESCMenu(const FInputActionValue& Value);

	// 현재 프레임의 WASD 입력 벡터를 저장 -> 구르기 시 방향 지정을 위한 변수
	FVector2D CurrentMovementInput;

// ================================================================================================================================================
// Interact Section
public:
	// 레이캐스트 거리
	UPROPERTY(EditDefaultsOnly, Category = "Interact")
	float InteractDistance = 250.f;

	// 서버 전용 setter (문/상점의 BeginOverlap에서 호출)
	FORCEINLINE void SetCurrentInteractable_ServerOnly(AActor* Target) 
	{ 
		ensureMsgf(HasAuthority(), TEXT("Server-only setter called on client"));
		CurrentInteractable = Target; 
	}
	FORCEINLINE void ClearCurrentInteractable_ServerOnly(AActor* Target)
	{
		ensureMsgf(HasAuthority(), TEXT("Server-only setter called on client"));
		if (CurrentInteractable == Target) CurrentInteractable = nullptr;
	}

	// 클라 UI용
	UFUNCTION(Client, Reliable) 
	void ClientRPC_SetCurrentInteractable(AActor* Target);
	
	UFUNCTION(Client, Reliable) 
	void ClientRPC_ClearCurrentInteractable(AActor* Target);

protected:
	UFUNCTION()	void OnInteract(const FInputActionValue& Value);
	
	// 현재 오버랩 중인 상호작용 대상(서버/클라 각각 유지)
	UPROPERTY() AActor* CurrentInteractable = nullptr;

	// 서버 RPC (플레이어는 항상 소유자가 보장됨)
	UFUNCTION(Server, Reliable)
	void ServerRPC_TryInteract();

// ================================================================================================================================================
// Roll Section
protected:
	void Roll();
	const float RollDistance = 263.0f;		// 구르는 거리, 필요하면 사용
	uint8 bCanRolling : 1;
	uint8 bIsRolling : 1;
	
	
	// 애니메이션 재생 함수(코드 분리)
	void PlayRollAnimation();

	// 구르기 몽타주 - 생성자 코드에서 연결됨
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	TObjectPtr<class UAnimMontage> RollActionMontage;

	// 구르기 서버 RPC - 서버 권위로 회피 판정
	UFUNCTION(Server, Reliable)
	void ServerRPCRoll(FVector_NetQuantize InDirection);

	void HandleStartRollTimer();

	void ComputeRollDirection(FVector& InDirection);
	bool CanRollEarly();
	void StartRollInvincibleTimer(const TWeakObjectPtr<ACGCharacterPlayer>& InWeakThis);
	void StartRollCooldownTimer(const TWeakObjectPtr<ACGCharacterPlayer>& InWeakThis);
	void StartRollControllerTimer(const TWeakObjectPtr<ACGCharacterPlayer>& InWeakThis);

public:
	// 다른 로컬 유저들도 자신의 구르는 모션이 진행되어야 함.
	UFUNCTION(NetMulticast, Reliable)
	void MultiCastRPCRoll(FVector_NetQuantizeNormal InRollDirection);
	

// ================================================================================================================================================
// 전투 시스템 Section
protected:
	void Attack();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	TObjectPtr<class UAnimMontage> ComboAttackActionMontage;

	UFUNCTION(Server, Reliable)
	void ServerRPCAttack(float AttackStartTime);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCAttack(int32 InCurrentCombo, FName InSection, float InRate);

	UFUNCTION(Server, Reliable)
	void ServerRPCNotifyHit(FVector_NetQuantize InStart, FVector_NetQuantize InEnd, const uint16 InAttackSequence, const uint8 InCurrentCombo, float HitCheckTime);

	UFUNCTION(Server, Reliable)
	void ServerRPCNotifyMiss(FVector_NetQuantize TraceStart, FVector_NetQuantize TraceEnd, FVector_NetQuantizeNormal TraceDir, float HitCheckTime);


	uint8 bIsAttack : 1;
	uint8 bCanRollEarly : 1;
	FTimerHandle CanRollEarly_Handle;

	void PlayAttackAnimation();
	virtual void AttackHitCheck() override;
	void AttackHitConfirm(AActor* HitActor);

	float AttackTime = 1.4667f;
	float LastAttackStartTime = 0.0f;
	float AttackTimeDifference = 0.0f;
	float AcceptCheckDistance = 300.0f;
	float AcceptMinCheckTime = 0.15f;	// 공격 간격 시간 최소 오차

	void DrawDebugAttackRange(const FColor& DrawColor, FVector TraceStart, FVector TraceEnd, FVector Forward);


	// 콤보 공격 ====================================================================================================
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCGActionData> ComboActionData;

	void ProcessComboCommand();

	void ComboActionBegin();
	void ComboActionEnd(class UAnimMontage* TargetMontage, bool IsProperlyEnded);
	void SetComboCheckTimer();
	void ComboCheck();

	uint8 CurrentCombo = 0;
	uint16 AttackSequence = 0;

	const float MaxStartError = 150.0f;	// 1.5m
	const float EndError = 25.0f;		// 0.25m 
	const float MinForwardCos = 0.5f;	// 60도

	FTimerHandle ComboTimerHandle;
	bool HasNextComboCommand = true;


	// Temp Variable
	UPROPERTY(EditAnywhere, Category = Fight, Meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class ACGCharacterMonster> OpponentClass;


	UPROPERTY(EditAnywhere, Category = Fight, Meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class ACGCharacterBossMonster> OpponentBossClass;

	UPROPERTY(EditAnywhere, Category = Fight, Meta = (AllowPrivateAccess = "true"))
	float OpponentSpawnTime;


// ================================================================================================================================================
// UI/HUD Section
protected:
	virtual void SetUpHUDWidget(class UCGHUDUserWidget* InHUDWidget);

// ================================================================================================================================================
// Weapon Section
protected:
	// 무기 착용
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCGWeaponComponent> WeaponComponent;

// Skin Change Section
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCGCharacterSkinComponent> SkinComponent;
};
