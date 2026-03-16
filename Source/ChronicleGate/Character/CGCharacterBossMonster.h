// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/CGCharacterBase.h"
#include "Interface/CGCharacterAIInterface.h"
#include "CGStatData.h"
#include "CGCharacterBossMonster.generated.h"

/**
 * 
 */
UCLASS()
class CHRONICLEGATE_API ACGCharacterBossMonster : public ACGCharacterBase, public ICGCharacterAIInterface
{
	GENERATED_BODY()
	
public:
	ACGCharacterBossMonster();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	FName BossName = TEXT("Grux2");
	FORCEINLINE FName GetBossName() { return BossName; }

protected:
	virtual void BeginPlay() override;




protected:
	void SetDead() override;

	UFUNCTION(NetMulticast, Reliable)
	void MultiCastDeadAnimation();
	void PlayDeadAnimation();


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Stat, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UAnimMontage> BossMonsterDeadMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	TObjectPtr<class UAnimMontage> BossMonsterAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	TObjectPtr<class UAnimMontage> RoarMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	TObjectPtr<class UAnimMontage> StartMontage;

// AI Section
protected:
	virtual float GetAIPatrolRadius() override;
	virtual float GetAIDetectRange() override;
	virtual float GetAIAttackRange() override;
	virtual float GetAITurnSpeed() override;

	virtual void SetAIAttackDelegate(const FAICharacterAttackFinished& InOnAttackFinished) override;
	virtual void AttackByAI() override;

	virtual void NotifyComboActionEnd() override;

	virtual void AttackHitCheck();
	void AttackHitConfirm(AActor* HitActor);
	UPROPERTY(EditAnywhere, Category = "Attack")
	float HitDelay = 38.f / 170.f;


	UFUNCTION(NetMulticast, Reliable)
	void MulticastAnimation();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	uint8 bInterBerserkMode = false;
	void BerserkMode();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastBerserkMode();
	uint8 bCanAttack = true;
	UFUNCTION()
	void OnBerserkDelayFinished();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStartSpawn();

// Boss Stat Section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossStat")
	FCGBaseStat BossMonsterStat;
};
