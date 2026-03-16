// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/CGCharacterMonster.h"
#include "Engine/AssetManager.h"
#include "AI/CGAIController.h"
#include "CGStatComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
//#include "Animation/AnimMontage.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/GameStateBase.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Game/CGStageStateBase.h"

ACGCharacterMonster::ACGCharacterMonster()
{
	AIControllerClass = ACGAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

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
	// 스켈레탈 매쉬 연결
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MonsterMeshRef(TEXT("/Script/Engine.SkeletalMesh'/Game/ROG_Creatures/Stickman/Meshes/SK_Stickman.SK_Stickman'"));
	if (MonsterMeshRef.Object)
	{
		GetMesh()->SetSkeletalMesh(MonsterMeshRef.Object);
	}
	// 애니메이션 연결
	static ConstructorHelpers::FClassFinder<UAnimInstance> MonsterAnimInstanceClassRef(TEXT("/Game/ROG_Creatures/Stickman/Animations/ABP_Stickman.ABP_Stickman_C"));
	if (MonsterAnimInstanceClassRef.Class)
	{
		GetMesh()->SetAnimInstanceClass(MonsterAnimInstanceClassRef.Class);
	}

	// MonsterAttackAction 몽타주 연결
	static ConstructorHelpers::FObjectFinder<UAnimMontage> MonsterAttackMontageRef(TEXT("/Script/Engine.AnimMontage'/Game/ROG_Creatures/Stickman/Animations/AM_Stickman_Attack_01.AM_Stickman_Attack_01'"));
	if (MonsterAttackMontageRef.Object)
	{
		MonsterAttackMontage = MonsterAttackMontageRef.Object;
	}

	// MonsterDeadAction 몽타주 연결
	static ConstructorHelpers::FObjectFinder<UAnimMontage> MonsterDeadMontageRef(TEXT("/Script/Engine.AnimMontage'/Game/ROG_Creatures/Stickman/Animations/AM_Stickman_Death.AM_Stickman_Death'"));
	if (MonsterDeadMontageRef.Object)
	{
		MonsterDeadMontage = MonsterDeadMontageRef.Object;
	}

	StatComponent = CreateDefaultSubobject<UCGStatComponent>(TEXT("StatComponent"));

	SetActorScale3D(FVector(1.5f, 1.5f, 1.5f));
	bSetNoHit = false;

	// 초기 몬스터 스탯 설계 -> 레벨 디자인으로 변경
	//MonsterStat.Attack = 1.0f;
	//MonsterStat.MaxHP = 100.f;
	//MonsterStat.AttackRange = 40.f;
	//MonsterStat.AttackSpeed = 1.0f;
	//MonsterStat.MovementSpeed = 400.f;
}

void ACGCharacterMonster::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void ACGCharacterMonster::BeginPlay()
{
	Super::BeginPlay();

	ACGStageStateBase* StageState = GetWorld()->GetGameState<ACGStageStateBase>();
	if (StatComponent && StageState)
	{
		int32 MonsterLevel = StageState->GetStageNum();
		StatComponent->SetLevelMonsterStat(MonsterLevel);

		UE_LOG(LogTemp, Log, TEXT("[Monster] AttackRange: %f"), StatComponent->GetTotalStat().AttackRange);
	}

	if (HasAuthority())
	{
		UE_LOG(LogTemp, Log, TEXT("[Monster] Authority. Controller=%s"), *GetNameSafe(GetController()));
		if (!GetController())
		{
			SpawnDefaultController();
			UE_LOG(LogTemp, Log, TEXT("[Monster] Forced SpawnDefaultController. Controller=%s"), *GetNameSafe(GetController()));
		}
	}

	if (HasAuthority())
	{
		if (StageState)
		{
			StageState->RegisterMonster(this);
		}
	}
}

void ACGCharacterMonster::SetDead()
{
	if (!HasAuthority() || bIsDead) return;
	bIsDead = true;
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	SetActorEnableCollision(false);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetGenerateOverlapEvents(false);
	//HpBar->SetHiddenInGame(true);
	ACGAIController* AIController = Cast<ACGAIController>(GetController());
	if (AIController)
	{
		AIController->StopAI();
	}
	if (ACGStageStateBase* StageState = GetWorld()->GetGameState<ACGStageStateBase>())
	{
		StageState->NotifyMonsterDied(this);
	}
	MultiCastDeadAnimation();
	SetLifeSpan(3.f);
}

void ACGCharacterMonster::MultiCastDeadAnimation_Implementation()
{
	PlayDeadAnimation();
	SetActorEnableCollision(false);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetGenerateOverlapEvents(false);
}

void ACGCharacterMonster::PlayDeadAnimation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->StopAllMontages(0.0f);
		AnimInstance->Montage_Play(MonsterDeadMontage);
	}
}

float ACGCharacterMonster::GetAIPatrolRadius()
{
	return 500.0f;
}

float ACGCharacterMonster::GetAIDetectRange()
{
	return 400.0f;
}

float ACGCharacterMonster::GetAIAttackRange()
{
	return StatComponent->GetTotalStat().AttackRange + StatComponent->GetAttackRadius() * 2;
}

float ACGCharacterMonster::GetAITurnSpeed()
{
	return 2.0f;
}

void ACGCharacterMonster::SetAIAttackDelegate(const FAICharacterAttackFinished& InOnAttackFinished)
{
	OnAttackFinished = InOnAttackFinished;
}

void ACGCharacterMonster::AttackByAI()
{
	if (HasAuthority())
	{
		FTimerHandle HitTimerHandle;
		TWeakObjectPtr<ACGCharacterMonster> WeakThis = this;
		GetWorld()->GetTimerManager().SetTimer(HitTimerHandle, FTimerDelegate::CreateLambda([WeakThis]
			{
				if (WeakThis.IsValid())
				{
					WeakThis->AttackHitCheck();
				}
			}
		), HitDelay, false);

		MulticastAnimation();
	}
}

void ACGCharacterMonster::NotifyComboActionEnd()
{
	Super::NotifyComboActionEnd();
	OnAttackFinished.ExecuteIfBound();
}

void ACGCharacterMonster::AttackHitCheck()
{
	if (HasAuthority())
	{
		FHitResult OutHitResult;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(Attack), false, this);

		const float AttackRange = StatComponent->GetTotalStat().AttackRange;
		const float AttackRadius = StatComponent->GetAttackRadius();
		const float AttackDamage = StatComponent->GetTotalStat().Attack;
		const FVector Forward = GetActorForwardVector();
		const FVector Start = GetActorLocation() + Forward * GetCapsuleComponent()->GetScaledCapsuleRadius();
		const FVector End = Start + GetActorForwardVector() * AttackRange;

		bool HitDetected = GetWorld()->SweepSingleByChannel(OutHitResult, Start, End, FQuat::Identity, ECC_GameTraceChannel1, FCollisionShape::MakeSphere(AttackRadius), Params);

		float HitCheckTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
		

		if (HitDetected)
		{
			AttackHitConfirm(OutHitResult.GetActor());
		}
	}
}

void ACGCharacterMonster::AttackHitConfirm(AActor* HitActor)
{
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Log, TEXT("[Server] AttackHitConfirm"));
		const float AttackDamage = StatComponent->GetTotalStat().Attack;
		FDamageEvent DamageEvent;
		HitActor->TakeDamage(AttackDamage, DamageEvent, GetController(), this);
	}
}

void ACGCharacterMonster::SetMonsterStat(int32 InStageLevel)
{
	StatComponent->SetLevelMonsterStat(InStageLevel);
}

void ACGCharacterMonster::MulticastAnimation_Implementation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->StopAllMontages(0.0f);
		AnimInstance->Montage_Play(MonsterAttackMontage);
		FTimerHandle Handle;
		TWeakObjectPtr<ACGCharacterMonster> WeakThis = this;
		GetWorld()->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([WeakThis]
			{
				if (WeakThis.IsValid())
				{
					WeakThis->NotifyComboActionEnd();
				}
			}), 2.0f, false);
		
	}
}
