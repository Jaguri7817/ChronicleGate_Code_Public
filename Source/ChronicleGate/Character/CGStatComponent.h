// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"		// 언리얼 네트워크, (예: 리플리케이션 지원)
#include "Character/CGStatData.h"	// 스탯 데이터
#include "CGStatComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnHPZeroDelegate);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnHPChangedDelegate, float);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnStatChangedDelegate, const FCGBaseStat& /*BaseStat*/, const FCGBaseStat& /*ModifierStat*/);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CHRONICLEGATE_API UCGStatComponent : public UActorComponent
{
	GENERATED_BODY()

// =================================================================================	
// 컴포넌트의 기본 설정 세팅 Section
public:	
	// 스탯 컴포넌트의 기본 설정들
	UCGStatComponent();

	// 리플리케이션할 변수 등록
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// 델리게이트 함수
	FOnHPZeroDelegate OnHPZero;				// HP가 0일때
	FOnHPChangedDelegate OnHPChanged;		// HP가 바뀔 때 -> HUD, UI 다시 그리기
	FOnStatChangedDelegate OnStatChanged;	// 스탯이 변경되면서 이펙트를 그려야할 때
	

protected:
	// 스탯 세팅, 이 클래스(컴포넌트)가 월드에 추가된 직후, 게임 시작 전에 호출
	// 스탯 초기화 진행
	virtual void InitializeComponent() override;

	// 델리게이트를 통해 플레이어 HUD 변경
	void BroadcastStatChanged();				

// =================================================================================
// 스탯 Section

protected:

	// 현재 플레이어의 체력, 0이 되면 죽은 것으로 간주
	UPROPERTY(Transient, ReplicatedUsing = OnRep_CurrentHP, VisibleInstanceOnly, Category = "Stat");
	float CurrentHP;
	
	// 공격 범위
	UPROPERTY(VisibleInstanceOnly, Category = Stat, Meta = (AllowPrivateAccess = "true"))
	float AttackRadius;

	// 공격 속도
	UPROPERTY(VisibleInstanceOnly, Category = Stat, Meta = (AllowPrivateAccess = "true"))
	float AttackSpeed;

	// 현재 플레이어의 베이스 스탯
	UPROPERTY(Transient, ReplicatedUsing = OnRep_BaseStat, VisibleInstanceOnly, Category = "Stat", meta = (AllowPrivateAccess = "true"))
	FCGBaseStat BaseStat;

	// 베이스 스탯에 더해질 추가 스탯
	UPROPERTY(Transient, ReplicatedUsing = OnRep_ModifierStat, VisibleInstanceOnly, Category = "Stat", meta = (AllowPrivateAccess = "true"))
	FCGBaseStat ModifierStat;

	// 현재 플레이어의 레벨
	UPROPERTY(Transient, Replicated, VisibleInstanceOnly, Category = "Stat")
	int32 CurrentLevel = 1;

	// -----------------------------------------------
	// 변수 변경에 대한 RepNotify
	UFUNCTION()	void OnRep_CurrentHP();
	UFUNCTION() void OnRep_BaseStat();
	UFUNCTION()	void OnRep_ModifierStat();
	
	

	// ====== 스탯 접근 게터 ======
public:
	FORCEINLINE float GetCurrentHP() const { return CurrentHP; }						// 현재 플레이터 HP 게터
	FORCEINLINE int32 GetCurrentLevel() const { return CurrentLevel; }					// 현재 플레이어 레벨 게터
	FORCEINLINE const FCGBaseStat& GetBaseStat() const { return BaseStat; }				// 베이스 스탯 게터
	FORCEINLINE const FCGBaseStat& GetModifierStat() const { return ModifierStat; }		// 추가 스탯 게터
	FORCEINLINE FCGBaseStat GetTotalStat() const { return BaseStat + ModifierStat; }	// 총합 스탯 게터
	FORCEINLINE float GetAttackRadius() const { return AttackRadius; }
	FORCEINLINE float GetAttackSpeed() const { return AttackSpeed; }

	// ===== 스탯 설정 세터 (서버 권위) =====
	FORCEINLINE void SetBaseStat(const FCGBaseStat& InBaseStat)
	{
		if (!GetOwner() || !GetOwner()->HasAuthority()) return;
		BaseStat = InBaseStat;
		SetHP(FMath::Clamp(CurrentHP, 0.f, GetTotalStat().MaxHP));			// MaxHp가 바뀌면 HP 클램프
	}
	FORCEINLINE void SetModifierStat(const FCGBaseStat& InModifierStat)
	{
		if (!GetOwner() || !GetOwner()->HasAuthority()) return;
		ModifierStat = InModifierStat;
		SetHP(FMath::Clamp(CurrentHP, 0.f, GetTotalStat().MaxHP));
	}
	void SetLevelStat(int32 InNewLevel);		// 레벨 프리셋 반영
	void SetLevelMonsterStat(int32 InStageLevel);
	void SetHP(float NewHP);					// HP 변경은 반드시 이 경로로만 (UI 이벤트/클램프/사망 처리 일원화)
	float ApplyDamage(float InDamage);

	// ====== API (서버 권위) ======
	FORCEINLINE void HealHP(float InHealAmount)			// 체력 회복
	{
		if (!GetOwner() || !GetOwner()->HasAuthority()) return;
		SetHP(FMath::Clamp(CurrentHP + InHealAmount, 0.f, GetTotalStat().MaxHP));
	}
};
