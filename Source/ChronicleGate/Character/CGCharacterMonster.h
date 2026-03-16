// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/CGCharacterBase.h"
#include "Engine/StreamableManager.h"
#include "Interface/CGCharacterAIInterface.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "CGCharacterMonster.generated.h"

/**
 * 
 */
UCLASS()
class CHRONICLEGATE_API ACGCharacterMonster : public ACGCharacterBase, public ICGCharacterAIInterface
{
	GENERATED_BODY()

public:
	ACGCharacterMonster();

protected:
	virtual void PostInitializeComponents() override;
	
	virtual void BeginPlay() override;

protected:
	void SetDead() override;
	UFUNCTION(NetMulticast, Reliable)
	void MultiCastDeadAnimation();
	void PlayDeadAnimation();
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Stat, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UAnimMontage> MonsterDeadMontage;

	UPROPERTY(config)
	TArray<FSoftObjectPath> NPCMeshes;
	TSharedPtr<FStreamableHandle> NPCMeshHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	TObjectPtr<class UAnimMontage> MonsterAttackMontage;


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
	float HitDelay = 22.f / 60.f;


	UFUNCTION(NetMulticast, Reliable)
	void MulticastAnimation();

	UPROPERTY(EditAnywhere)
	FCGBaseStat MonsterStat;


public:
	UFUNCTION()
	void SetMonsterStat(int32 InStageLevel);
};
