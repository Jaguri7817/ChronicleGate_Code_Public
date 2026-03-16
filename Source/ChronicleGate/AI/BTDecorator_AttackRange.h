// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_AttackRange.generated.h"

/**
 * 
 */
UCLASS()
class CHRONICLEGATE_API UBTDecorator_AttackRange : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UBTDecorator_AttackRange();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	
	
};
