// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/CGCharacterPlayer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "Controller/CGCharacterPlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/DamageEvents.h"
#include "ActionData/CGActionData.h"
#include "Kismet/KismetSystemLibrary.h"
#include "HUD/CGHUDUserWidget.h"
#include "Equipment/CGWeaponComponent.h"
#include "Engine/NetSerialization.h"
#include "Equipment/CGCharacterSkinComponent.h"
#include "Game/CGGameInstance.h"

#include "CGCharacterMonster.h"
#include "CGCharacterBossMonster.h"


// ================================================================================================================
// 기본 세팅 구현부
// ================================================================================================================

ACGCharacterPlayer::ACGCharacterPlayer()
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("CGCapsule"));

	// Movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Mesh
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -100.0f), FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));


	static ConstructorHelpers::FObjectFinder<USkeletalMesh> CharacterMeshRef(TEXT("/Script/Engine.SkeletalMesh'/Game/InfinityBladeWarriors/Character/CompleteCharacters/SK_CharM_Natural.SK_CharM_Natural'"));
	if (CharacterMeshRef.Object)
	{
		GetMesh()->SetSkeletalMesh(CharacterMeshRef.Object);
	}
	// 애니메이션 연결
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimInstanceClassRef(TEXT("/Game/ChronicleGate/Animation/ABP_CGCharacter.ABP_CGCharacter_C"));
	if (AnimInstanceClassRef.Class)
	{
		GetMesh()->SetAnimInstanceClass(AnimInstanceClassRef.Class);
	}
	
// 카메라 Section =============================================================================================================
	// 카메라 붐 생성 (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// 카메라 생성
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm


// 입력 Section =============================================================================================================

	// DMC 연결
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> InputMappingContextRef(TEXT("/Script/EnhancedInput.InputMappingContext'/Game/ChronicleGate/Input/IMC_Default.IMC_Default'"));
	if (InputMappingContextRef.Object)
	{
		DefaultMappingContext = InputMappingContextRef.Object;
	}

	// Move 연결
	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionMoveRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ChronicleGate/Input/Actions/IA_Move.IA_Move'"));
	if (InputActionMoveRef.Object)
	{
		MoveAction = InputActionMoveRef.Object;
	}

	// Look 연결
	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionLookRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ChronicleGate/Input/Actions/IA_Look.IA_Look'"));
	if (InputActionLookRef.Object)
	{
		LookAction = InputActionLookRef.Object;
	}

	// Roll 연결
	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionRollRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ChronicleGate/Input/Actions/IA_Roll.IA_Roll'"));
	if (InputActionRollRef.Object)
	{
		RollAction = InputActionRollRef.Object;
	}

	// RollAction 몽타주 연결
	static ConstructorHelpers::FObjectFinder<UAnimMontage> RollActionMontageRef(TEXT("/Script/Engine.AnimMontage'/Game/ChronicleGate/Animation/AM_RollAction.AM_RollAction'"));
	if (RollActionMontageRef.Object)
	{
		RollActionMontage = RollActionMontageRef.Object;
	}

	// Attack 연결
	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionAttackRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ChronicleGate/Input/Actions/IA_Attack.IA_Attack'"));
	if (InputActionAttackRef.Object)
	{
		AttackAction = InputActionAttackRef.Object;
	}

	// InteractAction 연결
	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionInteractActionRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ChronicleGate/Input/Actions/IA_Interact.IA_Interact'"));
	if (InputActionInteractActionRef.Object)
	{
		InteractAction = InputActionInteractActionRef.Object;
	}

	// ESCMenuAction 연결
	static ConstructorHelpers::FObjectFinder<UInputAction> ESCMenuActionRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ChronicleGate/Input/Actions/IA_ESCMenu.IA_ESCMenu'"));
	if (ESCMenuActionRef.Object)
	{
		ESCMenuAction = ESCMenuActionRef.Object;
	}

	// ComboAttackAction 몽타주 연결
	static ConstructorHelpers::FObjectFinder<UAnimMontage> ComboAttackActionMontageRef(TEXT("/Script/Engine.AnimMontage'/Game/ChronicleGate/Animation/AM_ComboAttackAction.AM_ComboAttackAction'"));
	if (ComboAttackActionMontageRef.Object)
	{
		ComboAttackActionMontage = ComboAttackActionMontageRef.Object;
	}
	// 콤보 공격 애니메이션 핸들링용 데이터 연결
	static ConstructorHelpers::FObjectFinder<UCGActionData> ComboActionDataRef(TEXT("/Script/ChronicleGate.CGActionData'/Game/ChronicleGate/Data/ComboActionData.ComboActionData'"));
	if (ComboActionDataRef.Object)
	{
		ComboActionData = ComboActionDataRef.Object;
	}


// 스탯 Section =============================================================================================================
	StatComponent = CreateDefaultSubobject<UCGStatComponent>(TEXT("StatComponent"));

// Roll Section =============================================================================================================
	bCanRolling = true;
	bIsRolling = false;
// Attack Section ===========================================================================================================
	bIsAttack = false;

// Weapon Section ===========================================================================================================
	WeaponComponent = CreateDefaultSubobject<UCGWeaponComponent>(TEXT("WeaponComponent"));

// Skin Change Section ======================================================================================================
	SkinComponent = CreateDefaultSubobject<UCGCharacterSkinComponent>(TEXT("SkinComponent"));
}

void ACGCharacterPlayer::BeginPlay()
{
	Super::BeginPlay();
#if !UE_SERVER
	// Add Input Mapping Context
	if (IsLocallyControlled())
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				if (DefaultMappingContext)
				{
					Subsystem->AddMappingContext(DefaultMappingContext, 0);
				}
			}
		
		}
	}
#endif
	if (IsLocallyControlled())
	{
		ACGCharacterBase* CharacterBase = Cast<ACGCharacterBase>(this);
		if (CharacterBase)
		{
			CharacterBase->SetHiddenHPBar();

		}
	}
	if (UCGGameInstance* GameInstance = GetWorld()->GetGameInstance<UCGGameInstance>())
	{
		FCGNFTSkinInfo SavedSkin;
		if (GameInstance->GetSelectedSkin(SavedSkin))
		{
			if (SkinComponent = FindComponentByClass<UCGCharacterSkinComponent>())
			{
				SkinComponent->ApplySkinFromNFT(SavedSkin);
				UE_LOG(LogTemp, Log, TEXT("[Character] Re-applied saved skin: %s"),	*SavedSkin.ItemCode);
			}
		}
	}
}

void ACGCharacterPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

#if !UE_SERVER
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	// Moving
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACGCharacterPlayer::Move);
	// Looking
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACGCharacterPlayer::Look);
	// Rolling
	EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Triggered, this, &ACGCharacterPlayer::Roll);
	// Attacking
	EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ACGCharacterPlayer::Attack);
	// Interacting
	EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ACGCharacterPlayer::OnInteract);
	// ESC Menu
	EnhancedInputComponent->BindAction(ESCMenuAction, ETriggerEvent::Started, this, &ACGCharacterPlayer::Input_ToggleESCMenu);
#endif
}

void ACGCharacterPlayer::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// 구르기 시 방향 지정
	CurrentMovementInput = MovementVector;

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.X);
		AddMovementInput(RightDirection, MovementVector.Y);
	}
	
}

void ACGCharacterPlayer::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ACGCharacterPlayer::Input_ToggleESCMenu(const FInputActionValue& Value)
{
	if (ACGCharacterPlayerController* CGPlayerController = Cast<ACGCharacterPlayerController>(GetController()))
	{
		if (CGPlayerController->IsLocalController())
		{
			UE_LOG(LogTemp, Warning, TEXT("[Player] Toggle ESC Menu"));
			CGPlayerController->ToggleESCMenu();
		}
	}
}

// ================================================================================================================
// 상호작용 구현부
// ================================================================================================================

void ACGCharacterPlayer::ClientRPC_SetCurrentInteractable_Implementation(AActor* Target) 
{
	CurrentInteractable = Target; 
}

void ACGCharacterPlayer::ClientRPC_ClearCurrentInteractable_Implementation(AActor* Target)
{
	if (CurrentInteractable == Target)
	{
		CurrentInteractable = nullptr;
	}
}

void ACGCharacterPlayer::OnInteract(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("[Player] OnInteract pressed E"));
	if (HasAuthority())
	{
		ServerRPC_TryInteract(); // 전용 서버에서도 동일 경로
	}
	else
	{
		ServerRPC_TryInteract(); // 클라 -> 서버 (소유자 RPC라 안전)
	}
}

void ACGCharacterPlayer::ServerRPC_TryInteract_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("[Player] Server_TryInteract: Target=%s"), *GetNameSafe(CurrentInteractable));
	AActor* Target = CurrentInteractable;

	// 상호작용할 타겟이 없으면(타겟 : LevelStreamer)
	if (!IsValid(Target))
	{
		UE_LOG(LogTemp, Error, TEXT("[Player] NO TARGET"));
		return;
	}

	// 상호작용할 타겟이 상호작용 인터페이스를 가지지 않으면(타겟 : LevelStreamer)
	if (!Target->GetClass()->ImplementsInterface(UCGInteractInterface::StaticClass()))
	{
		UE_LOG(LogTemp, Error, TEXT("[Player] Target has NO Interface"));
		return;
	}

	// 서버 재검증(옵션): CanInteract 한 번 더 확인
	/*
	if (!ICGInteractInterface::Execute_CanInteract(Target, this))
	{
		return;
	}
	*/

	// 타겟과 플레이어 간 상호작용 가능 여부 판단
	const bool bCan = ICGInteractInterface::Execute_CanInteract(Target, this);
	UE_LOG(LogTemp, Warning, TEXT("[Player] CanInteract? %d"), bCan);

	if (!bCan) return;

	UE_LOG(LogTemp, Warning, TEXT("[Player] Execute Interact"));
	ICGInteractInterface::Execute_Interact(Target, this);
}

// ================================================================================================================
// 구르기 스킬 구현부
// ================================================================================================================

void ACGCharacterPlayer::Roll()
{	
	FVector MoveDir = GetCharacterMovement()->GetLastInputVector(); // 마지막 WASD 방향

	// 살짝 안전장치
	MoveDir.Z = 0.f;
	if (MoveDir.IsNearlyZero())
	{
		// 아무 키도 안눌렀으면 정면으로
		MoveDir = GetActorForwardVector();
	}
	MoveDir.Normalize();

	
	ServerRPCRoll(MoveDir);
}

void ACGCharacterPlayer::PlayRollAnimation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && IsValid(AnimInstance))
	{
		AnimInstance->StopAllMontages(0.0f);			// 애니메이션 실행하기 전 모든 애니메이션 즉시(0.0f) 중지
		AnimInstance->Montage_Play(RollActionMontage);
	}
}

void ACGCharacterPlayer::HandleStartRollTimer()
{
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->bOrientRotationToMovement = false; // 이동 방향 회전 무시
	}

	TWeakObjectPtr<ACGCharacterPlayer> WeakThis = this;

	StartRollInvincibleTimer(WeakThis);
	StartRollCooldownTimer(WeakThis);
	StartRollControllerTimer(WeakThis);
}

void ACGCharacterPlayer::ComputeRollDirection(FVector& InDirection)
{
	InDirection.Z = 0.f;

	if (InDirection.IsNearlyZero())
	{
		InDirection = GetActorForwardVector();
	}
	InDirection.Normalize();
}

bool ACGCharacterPlayer::CanRollEarly()
{
	if (!bIsAttack)
	{
		return true;
	}

	if (bCanRollEarly)
	{
		bIsAttack = false;
		bCanRollEarly = false;

		GetWorld()->GetTimerManager().ClearTimer(CanRollEarly_Handle);
		GetWorld()->GetTimerManager().ClearTimer(ComboTimerHandle);
		return true;
	}

	return false;
}

void ACGCharacterPlayer::StartRollInvincibleTimer(const TWeakObjectPtr<ACGCharacterPlayer>& InWeakThis)
{
	bSetNoHit = true;

	FTimerHandle DodgeHandle;
	GetWorld()->GetTimerManager().SetTimer(DodgeHandle, FTimerDelegate::CreateLambda([InWeakThis]
		{
			if (InWeakThis.IsValid())
			{
				// 노히트 판정
				InWeakThis->bSetNoHit = false;
			}
		}
	), 0.7f, false);
}

void ACGCharacterPlayer::StartRollCooldownTimer(const TWeakObjectPtr<ACGCharacterPlayer>& InWeakThis)
{
	bCanRolling = false;
	bIsRolling = true;

	FTimerHandle CoolTime_Roll;
	GetWorld()->GetTimerManager().SetTimer(CoolTime_Roll, FTimerDelegate::CreateLambda([InWeakThis]
		{
			if (InWeakThis.IsValid())
			{
				InWeakThis->bCanRolling = true;
				InWeakThis->bIsRolling = false;
			}
		}
	), 1.0f, false);
}

void ACGCharacterPlayer::StartRollControllerTimer(const TWeakObjectPtr<ACGCharacterPlayer>& InWeakThis)
{
	FTimerHandle ControllerHandle;
	GetWorld()->GetTimerManager().SetTimer(ControllerHandle, FTimerDelegate::CreateLambda([InWeakThis]
		{
			if (InWeakThis.IsValid())
			{
				if (UCharacterMovementComponent* MovementComp = InWeakThis->GetCharacterMovement())
				{
					MovementComp->bOrientRotationToMovement = true; // 이동 방향 회전 무시
				}
			}
		}
	), 0.8f, false);
}

void ACGCharacterPlayer::ServerRPCRoll_Implementation(FVector_NetQuantize InDirection)
{
	if (!CanRollEarly())
	{
		return;
	}

	if (bCanRolling)
	{
		HandleStartRollTimer();

		FVector RollDirection = InDirection;
		ComputeRollDirection(RollDirection);
		
		SetActorRotation(RollDirection.Rotation());
		MultiCastRPCRoll(RollDirection);
	}
}

void ACGCharacterPlayer::MultiCastRPCRoll_Implementation(FVector_NetQuantizeNormal InRollDirection)
{
	SetActorRotation(InRollDirection.Rotation());
	PlayRollAnimation();
}

// ================================================================================================================
// 공격 구현부
// ================================================================================================================

void ACGCharacterPlayer::Attack()
{	
	ServerRPCAttack(GetWorld()->GetGameState()->GetServerWorldTimeSeconds());
	UE_LOG(LogTemp, Log, TEXT("[Attack]"));	
}

void ACGCharacterPlayer::ServerRPCAttack_Implementation(float AttackStartTime)
{
	if (bIsRolling)
	{
		return;
	}
	const float Now = GetWorld()->GetTimeSeconds(); // 서버 시간 신뢰
	bIsAttack = true;

	// 공격 후딜레이를 구르기로 캔슬(임시, MVP)
	TWeakObjectPtr<ACGCharacterPlayer> WeakThis = this;
	bCanRollEarly = false;
	GetWorld()->GetTimerManager().ClearTimer(CanRollEarly_Handle);
	GetWorld()->GetTimerManager().SetTimer(CanRollEarly_Handle, FTimerDelegate::CreateLambda([WeakThis]
		{
			if (WeakThis.IsValid())
			{
				WeakThis->bCanRollEarly = true;
			}
		}
	), 0.7f, false);

	if (CurrentCombo == 0)
	{
		if (LastAttackStartTime != 0.0f && (Now - LastAttackStartTime) <= AttackTime)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Server] ServerRPCAttack is Invalid Request"));
			return;
		}
		UE_LOG(LogTemp, Log, TEXT("[Server RPC Attack]"));
		ProcessComboCommand();        // 서버만 상태 변경
		LastAttackStartTime = Now;

		FName Section = *FString::Printf(TEXT("%s%d"), *ComboActionData->MontageSectionNamePrefix, CurrentCombo);
		MulticastRPCAttack(CurrentCombo, Section, StatComponent->GetTotalStat().AttackSpeed);         // 재생/점프는 멀티캐스트에서 통일 처리
		return;
	}
	
	if (ComboTimerHandle.IsValid())
	{
		HasNextComboCommand = true;

		return;
	}
	else
	{
		// 윈도우 밖: 거절(필요하면 관용 그레이스 타임 허용)
		UE_LOG(LogTemp, Warning, TEXT("[Server] Rejected: outside combo window"));
		return;
	}
}

void ACGCharacterPlayer::ServerRPCNotifyMiss_Implementation(FVector_NetQuantize TraceStart, FVector_NetQuantize TraceEnd, FVector_NetQuantizeNormal TraceDir, float HitCheckTime)
{
	if ((HitCheckTime - LastAttackStartTime) > AcceptMinCheckTime)
	{
		DrawDebugAttackRange(FColor::Red, TraceStart, TraceEnd, TraceDir);
	}

}

void ACGCharacterPlayer::ServerRPCNotifyHit_Implementation(FVector_NetQuantize InStart, FVector_NetQuantize InEnd, const uint16 InAttackSequence, const uint8 InCurrentCombo, float HitCheckTime)
{
	if ((HitCheckTime - LastAttackStartTime) < AcceptMinCheckTime) return;
	
	FHitResult ServerHitResult;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(Attack), false, this);

	const float AttackRange = StatComponent->GetTotalStat().AttackRange;
	const float AttackRadius = StatComponent->GetAttackRadius();

	const FVector ClientStart = FVector(InStart);
	const FVector ClientEnd = FVector(InEnd);

	const FVector ServerStart = GetActorLocation() + GetActorForwardVector() * GetCapsuleComponent()->GetScaledCapsuleRadius();
		
	// Start 허용 오차 & 치트 방지
	if (FVector::DistSquared(ServerStart, ClientStart) > FMath::Square(MaxStartError)) return;
		
	// End 허용 오차 & 치트 방지
	if (FVector::DistSquared(ClientStart, ClientEnd) > FMath::Square(AttackRange + EndError)) return;

	// 방향(각도) 오차 & 치트 방지
	const float Dot = FVector::DotProduct(GetActorForwardVector(), (ClientEnd - ClientStart).GetSafeNormal());
	if (Dot < MinForwardCos) return;


	const bool bServerHitDetected = GetWorld()->SweepSingleByChannel(ServerHitResult, ClientStart, ClientEnd, FQuat::Identity, ECC_GameTraceChannel1, FCollisionShape::MakeSphere(AttackRadius), Params);

	if (!bServerHitDetected) return;

	AActor* HitActor = ServerHitResult.GetActor();
	if (::IsValid(HitActor))
	{
		const FVector HitLocation = ServerHitResult.ImpactPoint;
		const FBox HitBox = HitActor->GetComponentsBoundingBox();
		const FVector ActorBoxCenter = (HitBox.Min + HitBox.Max) * 0.5f;
		if (FVector::DistSquared(HitLocation, ActorBoxCenter) <= AcceptCheckDistance * AcceptCheckDistance)
		{
			AttackHitConfirm(HitActor);
		}
#if ENABLE_DRAW_DEBUG
		DrawDebugPoint(GetWorld(), ActorBoxCenter, 50.0f, FColor::Cyan, false, 5.0f);
		DrawDebugPoint(GetWorld(), HitLocation, 50.0f, FColor::Magenta, false, 5.0f);
#endif
		const FVector Forward = (ClientEnd - ClientStart).GetSafeNormal();
		DrawDebugAttackRange(FColor::Green, ClientStart, ClientEnd, Forward);
	}
	
}

void ACGCharacterPlayer::MulticastRPCAttack_Implementation(int32 InCurrentCombo, FName InSection, float InRate)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance || !ComboAttackActionMontage)
	{
		return;
	}

	const float AttackSpeedRate = InRate;
	const FName Section = InSection;
	CurrentCombo = InCurrentCombo;
	
	if (CurrentCombo == 1)
	{
		AnimInstance->Montage_Play(ComboAttackActionMontage, AttackSpeedRate);

		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &ACGCharacterPlayer::ComboActionEnd);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, ComboAttackActionMontage);
	}
	else
	{
		AnimInstance->Montage_JumpToSection(Section, ComboAttackActionMontage);
		AnimInstance->Montage_SetPlayRate(ComboAttackActionMontage, AttackSpeedRate);
	}
	
	UE_LOG(LogTemp, Log, TEXT("[MultiCast Attack]"));
}

void ACGCharacterPlayer::PlayAttackAnimation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	AnimInstance->StopAllMontages(0.0f);
	AnimInstance->Montage_Play(ComboAttackActionMontage);
}

void ACGCharacterPlayer::AttackHitCheck()
{
	if (IsLocallyControlled())
	{
		FHitResult OutHitResult;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(Attack), false, this);

		const float AttackRange = StatComponent->GetTotalStat().AttackRange;
		const float AttackRadius = StatComponent->GetAttackRadius();
		
		const FVector Forward = GetActorForwardVector();
		const FVector Start = GetActorLocation() + Forward * GetCapsuleComponent()->GetScaledCapsuleRadius();
		const FVector End = Start + GetActorForwardVector() * AttackRange;

		bool HitDetected = GetWorld()->SweepSingleByChannel(OutHitResult, Start, End, FQuat::Identity, ECC_GameTraceChannel1, FCollisionShape::MakeSphere(AttackRadius), Params);

		float HitCheckTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
		if (!HasAuthority())
		{
			if (HitDetected)
			{
				ServerRPCNotifyHit(Start, End, AttackSequence, CurrentCombo, HitCheckTime);
			}
			else
			{
				ServerRPCNotifyMiss(Start, End, Forward, HitCheckTime);
			}
		}
	}
}

void ACGCharacterPlayer::AttackHitConfirm(AActor* HitActor)
{
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Log, TEXT("[Server] AttackHitConfirm"));
		const float AttackDamage = StatComponent->GetTotalStat().Attack;
		FDamageEvent DamageEvent;
		HitActor->TakeDamage(AttackDamage, DamageEvent, GetController(), this);

		float Damage = AttackDamage;
		UE_LOG(LogTemp, Warning, TEXT("[HitConfirm] Attack=%f, Damage=%f"), AttackDamage, Damage);
	}
}

void ACGCharacterPlayer::DrawDebugAttackRange(const FColor& DrawColor, FVector TraceStart, FVector TraceEnd, FVector Forward)
{
#if ENABLE_DRAW_DEBUG

	const float AttackRange = StatComponent->GetTotalStat().AttackRange;
	const float AttackRadius = StatComponent->GetAttackRadius();
	FVector CapsuleOrigin = TraceStart + (TraceEnd - TraceStart) * 0.5f;
	float CapsuleHalfHeight = AttackRange * 0.5f;
	DrawDebugCapsule(GetWorld(), CapsuleOrigin, CapsuleHalfHeight, AttackRadius, FRotationMatrix::MakeFromZ(Forward).ToQuat(), DrawColor, false, 5.0f);

#endif
}
// ================================================================================================================
// 콤보 공격 구현부
// ================================================================================================================
void ACGCharacterPlayer::ProcessComboCommand()
{
	if (HasAuthority())
	{
		if (CurrentCombo == 0)
		{
			ComboActionBegin();
			return;
		}

		if (!ComboTimerHandle.IsValid())
		{
			HasNextComboCommand = false;
		}
		else
		{
			HasNextComboCommand = true;
		}
	}
	
}

void ACGCharacterPlayer::ComboActionBegin()
{
	CurrentCombo = 1;
	HasNextComboCommand = false;
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

	ComboTimerHandle.Invalidate();
	SetComboCheckTimer();
}

void ACGCharacterPlayer::ComboActionEnd(UAnimMontage* TargetMontage, bool IsProperlyEnded)
{
	ensure(CurrentCombo != 0);
	CurrentCombo = 0;
	bIsAttack = false;
	HasNextComboCommand = false;
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);

	NotifyComboActionEnd();
}

void ACGCharacterPlayer::SetComboCheckTimer()
{
	int32 ComboIndex = CurrentCombo - 1;
	ensure(ComboActionData->EffectiveFrameCount.IsValidIndex(ComboIndex));

	const float AttackSpeedRate = StatComponent->GetTotalStat().AttackSpeed;
	float ComboEffectiveTime = (ComboActionData->EffectiveFrameCount[ComboIndex] / ComboActionData->FrameRate) / AttackSpeedRate;
	if (ComboEffectiveTime > 0.0f)
	{
		TWeakObjectPtr<ACGCharacterPlayer> WeakThis = this;
		GetWorld()->GetTimerManager().SetTimer(ComboTimerHandle, FTimerDelegate::CreateLambda([WeakThis]
			{
				if (WeakThis.IsValid())
				{
					WeakThis->ComboCheck();
				}
			}
		), ComboEffectiveTime, false);
	}
}

void ACGCharacterPlayer::ComboCheck()
{
	ComboTimerHandle.Invalidate();
	if (HasNextComboCommand)
	{
		CurrentCombo = FMath::Clamp(CurrentCombo + 1, 1, ComboActionData->MaxComboCount);

		SetComboCheckTimer();
		HasNextComboCommand = false;

		FName Section = *FString::Printf(TEXT("%s%d"), *ComboActionData->MontageSectionNamePrefix, CurrentCombo);
		MulticastRPCAttack(CurrentCombo, Section, StatComponent->GetTotalStat().AttackSpeed); // 섹션 점프 포함 재생/동기화는 여기서
	}
}

void ACGCharacterPlayer::SetUpHUDWidget(UCGHUDUserWidget* InHUDWidget)
{
	if (InHUDWidget)
	{
		float HP = StatComponent->GetTotalStat().MaxHP;
		InHUDWidget->SetMaxHP(HP);
		InHUDWidget->UpdateHPBar(StatComponent->GetCurrentHP());
		StatComponent->OnHPChanged.AddUObject(InHUDWidget, &UCGHUDUserWidget::UpdateHPBar);
	}
}
