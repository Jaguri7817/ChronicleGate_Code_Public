// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/CGCharacterBossMonster.h"
#include "Engine/AssetManager.h"
#include "AI/CGAIController.h"
#include "CGStatComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/GameStateBase.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Character/CGStatData.h"	// 스탯 데이터
#include "Game/CGStageStateBase.h"
#include "BrainComponent.h"


ACGCharacterBossMonster::ACGCharacterBossMonster()
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
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> BossMonsterMeshRef(TEXT("/Script/Engine.SkeletalMesh'/Game/ParagonGrux/Characters/Heroes/Grux/Meshes/Grux.Grux'"));
	if (BossMonsterMeshRef.Object)
	{
		GetMesh()->SetSkeletalMesh(BossMonsterMeshRef.Object);
	}
	// 애니메이션 연결
	static ConstructorHelpers::FClassFinder<UAnimInstance> BossMonsterAnimInstanceClassRef(TEXT("/Game/ParagonGrux/Characters/Heroes/Grux/Grux_AnimBlueprint.Grux_AnimBlueprint_C"));
	if (BossMonsterAnimInstanceClassRef.Class)
	{
		GetMesh()->SetAnimInstanceClass(BossMonsterAnimInstanceClassRef.Class);
	}

	// BossMonsterAttackAction 몽타주 연결
	static ConstructorHelpers::FObjectFinder<UAnimMontage> BossMonsterAttackMontageRef(TEXT("/Script/Engine.AnimMontage'/Game/ParagonGrux/Characters/Heroes/Grux/Animations/PrimaryAttack_LA_Montage.PrimaryAttack_LA_Montage'"));
	if (BossMonsterAttackMontageRef.Object)
	{
		BossMonsterAttackMontage = BossMonsterAttackMontageRef.Object;
	}

	// BossMonsterDeadAction 몽타주 연결
	static ConstructorHelpers::FObjectFinder<UAnimMontage> BossMonsterDeadMontageRef(TEXT("/Script/Engine.AnimMontage'/Game/ParagonGrux/Characters/Heroes/Grux/Animations/AM_Grux_Dead.AM_Grux_Dead'"));
	if (BossMonsterDeadMontageRef.Object)
	{
		BossMonsterDeadMontage = BossMonsterDeadMontageRef.Object;
	}

	// RoarAction 몽타주 연결
	static ConstructorHelpers::FObjectFinder<UAnimMontage> RoarMontageRef(TEXT("/Script/Engine.AnimMontage'/Game/ParagonGrux/Characters/Heroes/Grux/Animations/AM_Roar.AM_Roar'"));
	if (RoarMontageRef.Object)
	{
		RoarMontage = RoarMontageRef.Object;
	}

	// StartAction 몽타주 연결
	static ConstructorHelpers::FObjectFinder<UAnimMontage> StartMontageRef(TEXT("/Script/Engine.AnimMontage'/Game/ParagonGrux/Characters/Heroes/Grux/Animations/LevelStart_Montage.LevelStart_Montage'"));
	if (StartMontageRef.Object)
	{
		StartMontage = StartMontageRef.Object;
	}

	StatComponent = CreateDefaultSubobject<UCGStatComponent>(TEXT("StatComponent"));

	SetActorScale3D(FVector(1.5f, 1.5f, 1.5f));
	bSetNoHit = false;
// Boss Stat
	BossMonsterStat.Attack = 5.0f;
	BossMonsterStat.MaxHP = 400.0f;
	BossMonsterStat.AttackRange = 40.0f;
	BossMonsterStat.AttackSpeed = 0.8f;
	BossMonsterStat.MovementSpeed = 400.0f;

}

void ACGCharacterBossMonster::BeginPlay()
{
	Super::BeginPlay();

	ACGStageStateBase* StageState = GetWorld()->GetGameState<ACGStageStateBase>();
	if (StatComponent && StageState)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Boss] BossMonsterStat MaxHP=%f"), BossMonsterStat.MaxHP);

		StatComponent->SetBaseStat(BossMonsterStat);
		StatComponent->SetHP(BossMonsterStat.MaxHP);
		UE_LOG(LogTemp, Warning, TEXT("[Boss] After SetBaseStat: HP=%f / Max=%f (Auth=%d)"), StatComponent->GetCurrentHP(),	StatComponent->GetTotalStat().MaxHP, HasAuthority());
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

void ACGCharacterBossMonster::SetDead()
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
	SetLifeSpan(5.f);
}

void ACGCharacterBossMonster::MultiCastDeadAnimation_Implementation()
{
	PlayDeadAnimation();
	SetActorEnableCollision(false);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetGenerateOverlapEvents(false);
}

void ACGCharacterBossMonster::PlayDeadAnimation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->StopAllMontages(0.0f);
		AnimInstance->Montage_Play(BossMonsterDeadMontage);
	}
}

float ACGCharacterBossMonster::GetAIPatrolRadius()
{
    return 500.0f;
}

float ACGCharacterBossMonster::GetAIDetectRange()
{
    return 400.0f;
}

float ACGCharacterBossMonster::GetAIAttackRange()
{
	return StatComponent->GetTotalStat().AttackRange + StatComponent->GetAttackRadius() * 2;
}

float ACGCharacterBossMonster::GetAITurnSpeed()
{
    return 2.0f;
}

void ACGCharacterBossMonster::SetAIAttackDelegate(const FAICharacterAttackFinished& InOnAttackFinished)
{
	OnAttackFinished = InOnAttackFinished;
}

void ACGCharacterBossMonster::AttackByAI()
{
	if (!bCanAttack) return;
	if (HasAuthority())
	{
		FTimerHandle HitTimerHandle;
		TWeakObjectPtr<ACGCharacterBossMonster> WeakThis = this;
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

void ACGCharacterBossMonster::NotifyComboActionEnd()
{
	Super::NotifyComboActionEnd();
	OnAttackFinished.ExecuteIfBound();
}


void ACGCharacterBossMonster::AttackHitCheck()
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

void ACGCharacterBossMonster::AttackHitConfirm(AActor* HitActor)
{
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Log, TEXT("[Server] AttackHitConfirm"));
		const float AttackDamage = StatComponent->GetTotalStat().Attack;
		FDamageEvent DamageEvent;
		HitActor->TakeDamage(AttackDamage, DamageEvent, GetController(), this);
	}
}

float ACGCharacterBossMonster::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority())
	{
		return 0.f;
	}
	if (bSetNoHit)
	{
		return 0.f;
	}

	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	//StatComponent->ApplyDamage(DamageAmount); 이미 데미지를 입음.
	const float InterBerserkMode = StatComponent->GetTotalStat().MaxHP * 0.4f;
	if (StatComponent->GetCurrentHP() <= InterBerserkMode && !bInterBerserkMode)
	{
		bInterBerserkMode = true;
		bCanAttack = false;
		MulticastBerserkMode();
		FTimerHandle BerserkTimerHandle;
		TWeakObjectPtr<ACGCharacterBossMonster> WeakThis = this;
		GetWorld()->GetTimerManager().SetTimer(BerserkTimerHandle, FTimerDelegate::CreateLambda([WeakThis]
			{
				if (WeakThis.IsValid())
				{
					WeakThis->OnBerserkDelayFinished();
				}
			}
		), 2.5f, false);
	}
	return DamageAmount;
}

void ACGCharacterBossMonster::BerserkMode()
{
	// 기존 스탯을 가져옵니다.
	FCGBaseStat CurrentBaseStat = StatComponent->GetBaseStat();

	// 새로운 스탯을 계산합니다.
	FCGBaseStat Berserk;
	FCGBaseStat BerserkStat;
	BerserkStat.MaxHP = CurrentBaseStat.MaxHP;
	BerserkStat.Attack = CurrentBaseStat.Attack * 1.5f;        // 공격력 1.5배
	BerserkStat.MovementSpeed = CurrentBaseStat.MovementSpeed; // 이동 속도는 그대로
	BerserkStat.AttackSpeed = CurrentBaseStat.AttackSpeed * 1.2f;  // 공격 속도 1.2배
	BerserkStat.AttackRange = CurrentBaseStat.AttackRange;     // 공격 사거리 그대로

	// 계산된 새로운 스탯을 스탯 컴포넌트에 적용합니다.
	StatComponent->SetBaseStat(BerserkStat);
}

void ACGCharacterBossMonster::OnBerserkDelayFinished()
{
	BerserkMode();
	bCanAttack = true;
	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		if (UBrainComponent* Brain = AI->GetBrainComponent())
		{
			Brain->RestartLogic();   // Behavior Tree 다시 돌리기
		}
	}
}

void ACGCharacterBossMonster::MulticastStartSpawn_Implementation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->StopAllMontages(0.0f);
		AnimInstance->Montage_Play(StartMontage);
	}
}

void ACGCharacterBossMonster::MulticastBerserkMode_Implementation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->StopAllMontages(0.0f);
		AnimInstance->Montage_Play(RoarMontage, 0.5f);
	}
}

void ACGCharacterBossMonster::MulticastAnimation_Implementation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->StopAllMontages(0.0f);
		AnimInstance->Montage_Play(BossMonsterAttackMontage);
		FTimerHandle Handle;
		TWeakObjectPtr<ACGCharacterBossMonster> WeakThis = this;
		GetWorld()->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([WeakThis]
			{
				if (WeakThis.IsValid())
				{
					WeakThis->NotifyComboActionEnd();
				}
			}
		), 2.0f, false);

	}
}
