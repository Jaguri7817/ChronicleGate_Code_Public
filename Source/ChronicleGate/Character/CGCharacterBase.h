// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Character/CGStatComponent.h"
#include "Interface/CGCharacterWidgetInterface.h"
#include "CGCharacterBase.generated.h"



UCLASS()
class CHRONICLEGATE_API ACGCharacterBase : public ACharacter, public ICGCharacterWidgetInterface
{
	GENERATED_BODY()

public:
	ACGCharacterBase();
	

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


// Ω∫≈» Section
protected:
	TObjectPtr<UCGStatComponent> StatComponent;

// ¿ß¡¨ Section
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Stat, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCGWidgetComponent> HPBar;

	virtual void SetUpCharacterWidget(class UCGUserWidget* InUserWidget);

public:
	void SetHiddenHPBar();
// Dead Section ================================================================================================
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Stat, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UAnimMontage> DeadMontage;

	virtual void PlayDeadAnimation();

	float DeadEventDelayTime = 5.0f;
	UPROPERTY(ReplicatedUsing = OnRep_IsDead)
	uint8 bIsDead : 1;

	UFUNCTION()
	void OnRep_IsDead();

public:
	virtual void SetDead();
	bool GetbIsDead();


// Attack Hit Section ============================================================================================
protected:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	uint8 bSetNoHit : 1;
public:
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCDead();
	virtual void NotifyComboActionEnd();

	
};
