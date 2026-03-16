// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/CGAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/CGCharacterPlayer.h"



UCGAnimInstance::UCGAnimInstance()
{
	MovingThreshould = 3.0f;
	bDead = false;
}

void UCGAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Owner = Cast<ACharacter>(GetOwningActor());
	if (Owner)
	{
		Movement = Owner->GetCharacterMovement();
	}
}

void UCGAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (Movement)
	{
		Velocity = Movement->Velocity;
		GroundSpeed = Velocity.Size2D();
		bIsIdle = GroundSpeed < MovingThreshould;
	}
	if (ACGCharacterBase* Character = Cast<ACGCharacterBase>(TryGetPawnOwner()))
	{
		bDead = Character->GetbIsDead();
	}
}
