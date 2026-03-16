// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/CGCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ActionData/CGActionData.h"
#include "Controller/CGCharacterPlayerController.h"
#include "Game/CGStageStateBase.h"
#include "HUD/CGWidgetComponent.h"
#include "HUD/CGHPBarWidget.h"


ACGCharacterBase::ACGCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<UAnimMontage> DeadActionMontageRef(TEXT("/Script/Engine.AnimMontage'/Game/ChronicleGate/Animation/AM_DeadAction.AM_DeadAction'"));
	if (DeadActionMontageRef.Object)
	{
		DeadMontage = DeadActionMontageRef.Object;
	}

	// Widget Component
	HPBar = CreateDefaultSubobject<UCGWidgetComponent>(TEXT("Widget"));
	HPBar->SetupAttachment(GetMesh());
	HPBar->SetRelativeLocation(FVector(0.0f, 0.0f, 180.0f));
	static ConstructorHelpers::FClassFinder<UUserWidget> HPBarWidgetRef(TEXT("/Game/HUD/WBP_CGHPBar.WBP_CGHPBar_C"));
	if (HPBarWidgetRef.Class)
	{
		HPBar->SetWidgetClass(HPBarWidgetRef.Class);
		HPBar->SetWidgetSpace(EWidgetSpace::Screen);
		HPBar->SetDrawSize(FVector2D(150.0f, 15.0f));
		HPBar->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	}
}

void ACGCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	bIsDead = false;
	SetActorEnableCollision(true);
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->StopAllMontages(0.0f);
	}
	if (StatComponent)
	{
		StatComponent->OnHPZero.AddUObject(this, &ACGCharacterBase::SetDead);
	}
	
}

void ACGCharacterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void ACGCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACGCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ACGCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACGCharacterBase, bIsDead);
}

void ACGCharacterBase::SetDead()
{
	if (!HasAuthority() || bIsDead) return;
	bIsDead = true;
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	SetActorEnableCollision(false);
	//HpBar->SetHiddenInGame(true);
	MulticastRPCDead();

	ACGStageStateBase* StageState = GetWorld()->GetGameState<ACGStageStateBase>();
	if (!StageState) return;
	StageState->OnPlayerDead();

	SetLifeSpan(5.f);
}

bool ACGCharacterBase::GetbIsDead()
{
	return bIsDead;
}

void ACGCharacterBase::OnRep_IsDead()
{
	if (bIsDead)
	{
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
		if (IsLocallyControlled())
		{
			DisableInput(GetController<ACGCharacterPlayerController>());
		}
	}
	else
	{
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
		if (IsLocallyControlled())
		{
			EnableInput(GetController<ACGCharacterPlayerController>());
		}
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			AnimInstance->StopAllMontages(0.0f);
		}
	}
	
}

void ACGCharacterBase::SetUpCharacterWidget(UCGUserWidget* InUserWidget)
{
	UCGHPBarWidget* HPBarWidget = Cast<UCGHPBarWidget>(InUserWidget);
	if (HPBarWidget)
	{
		HPBarWidget->SetMaxHP(StatComponent->GetTotalStat().MaxHP);							
		HPBarWidget->UpdateHPBar(StatComponent->GetCurrentHP());							 
		StatComponent->OnHPChanged.AddUObject(HPBarWidget, &UCGHPBarWidget::UpdateHPBar);	// HP가 변경될 때마다 델리게이트로 UpdateHPBar() 호출
	}
}

void ACGCharacterBase::SetHiddenHPBar()
{
	if (HPBar)
	{
		HPBar->SetHiddenInGame(true);
	}
}

void ACGCharacterBase::PlayDeadAnimation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->StopAllMontages(0.0f);
		AnimInstance->Montage_Play(DeadMontage, 1.0f);
	}
}

float ACGCharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
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
	StatComponent->ApplyDamage(DamageAmount);
	UE_LOG(LogTemp, Log, TEXT("[Attacker] %s / [Taker's HP] %f"), *DamageCauser->GetName(), StatComponent->GetCurrentHP());
	return DamageAmount;
}

void ACGCharacterBase::NotifyComboActionEnd()
{
}

void ACGCharacterBase::MulticastRPCDead_Implementation()
{
	PlayDeadAnimation();
}

