// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/CGStatComponent.h"
#include "GameFramework/Actor.h"
#include "Game/CGGameSingleton.h"
#include "Character/CGCharacterBase.h"


UCGStatComponent::UCGStatComponent()
{
	
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	// 컴포넌트 기본 설정 : InitializeComponent()를 호출하도록 설정
	bWantsInitializeComponent = true;

}

void UCGStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(UCGStatComponent, CurrentHP);
	//DOREPLIFETIME_CONDITION(UCGStatComponent, BaseStat, COND_AutonomousOnly);
	//DOREPLIFETIME_CONDITION(UCGStatComponent, ModifierStat, COND_AutonomousOnly);
	//DOREPLIFETIME_CONDITION(UCGStatComponent, CurrentLevel, COND_AutonomousOnly);
	DOREPLIFETIME(UCGStatComponent, BaseStat);
	DOREPLIFETIME(UCGStatComponent, ModifierStat);
	DOREPLIFETIME(UCGStatComponent, CurrentLevel);
}

void UCGStatComponent::InitializeComponent()
{
	Super::InitializeComponent();
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		CurrentLevel = 1;
		SetLevelStat(CurrentLevel);
		SetHP(GetTotalStat().MaxHP);
		AttackRadius = 50.0f;
	}
}

void UCGStatComponent::BroadcastStatChanged()
{
	OnStatChanged.Broadcast(BaseStat, ModifierStat);
}

void UCGStatComponent::OnRep_CurrentHP()
{
	OnHPChanged.Broadcast(CurrentHP);
}

void UCGStatComponent::OnRep_BaseStat()
{
	// MaxHP가 변했을 수 있으므로 HP를 안전하게 클램프
	if (CurrentHP > GetTotalStat().MaxHP)
	{
		CurrentHP = GetTotalStat().MaxHP;
		OnHPChanged.Broadcast(CurrentHP);
	}
	BroadcastStatChanged();
}

void UCGStatComponent::OnRep_ModifierStat()
{
	if (CurrentHP > GetTotalStat().MaxHP)
	{
		CurrentHP = GetTotalStat().MaxHP;
		OnHPChanged.Broadcast(CurrentHP);
	}
	BroadcastStatChanged();
}

void UCGStatComponent::SetLevelStat(int32 InNewLevel)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}
	if (InNewLevel < CurrentLevel)	// 레벨이 같아도 세팅되게끔 설정
	{
		return;
	}
	
	CurrentLevel = FMath::Clamp(InNewLevel, 1, UCGGameSingleton::Get().CharacterMaxLevel);
	SetBaseStat(UCGGameSingleton::Get().GetCharacterStat(CurrentLevel));
	//check(BaseStat.MaxHp > 0.0f);

	SetHP(GetTotalStat().MaxHP);
	
}

void UCGStatComponent::SetLevelMonsterStat(int32 InStageLevel)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	CurrentLevel = FMath::Clamp(InStageLevel, 1, UCGGameSingleton::Get().MonsterMaxLevel);
	SetBaseStat(UCGGameSingleton::Get().GetMonsterStat(CurrentLevel));
	//check(BaseStat.MaxHp > 0.0f);

	SetHP(GetTotalStat().MaxHP);
}

void UCGStatComponent::SetHP(float NewHP)
{
	const float OldHp = CurrentHP;
	CurrentHP = NewHP;

	// 서버/클라 모두에서 델리게이트 호출 (Listen Server 테스트/HUD 갱신 편의)
	if (!FMath::IsNearlyEqual(OldHp, CurrentHP))
	{
		OnHPChanged.Broadcast(CurrentHP);
	}
}

float UCGStatComponent::ApplyDamage(float InDamage)
{
	if (!GetOwner()->HasAuthority()) return 0.f;
	const float PrevHp = CurrentHP;
	const float ActualDamage = FMath::Clamp<float>(InDamage, 0, InDamage);

	UE_LOG(LogTemp, Warning, TEXT("[ApplyDamage] Damage=%f, OldHP=%f -> ActualDamage=%f"),	InDamage, PrevHp, ActualDamage);

	SetHP(PrevHp - ActualDamage);
	if (CurrentHP <= KINDA_SMALL_NUMBER)
	{
		//OnHPZero.Broadcast();
		ACGCharacterBase* Character = Cast<ACGCharacterBase>(GetOwner());
		if (Character)
		{
			Character->SetDead();
		}
	}
	return ActualDamage;
}


