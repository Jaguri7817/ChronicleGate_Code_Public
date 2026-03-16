// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "CGStatData.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FCGBaseStat : public FTableRowBase
{
	GENERATED_BODY()

public:
	FCGBaseStat() : MaxHP(0.0f), Attack(0.0f), MovementSpeed(0.0f), AttackSpeed(0.0f), AttackRange(0.0f) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat)
	float MaxHP;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat)
	float Attack;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat)
	float MovementSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat)
	float AttackSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat)
	float AttackRange;

	
	FORCEINLINE FCGBaseStat operator+(const FCGBaseStat& Other) const
	{
		FCGBaseStat R;
		R.MaxHP = MaxHP + Other.MaxHP;
		R.Attack = Attack + Other.Attack;
		R.MovementSpeed = MovementSpeed + Other.MovementSpeed;
		R.AttackSpeed = AttackSpeed + Other.AttackSpeed;
		R.AttackRange = AttackRange + Other.AttackRange;
		return R;
	}

	FORCEINLINE FCGBaseStat& operator+=(const FCGBaseStat& Other)
	{
		MaxHP += Other.MaxHP;
		Attack += Other.Attack;
		MovementSpeed += Other.MovementSpeed;
		AttackSpeed += Other.AttackSpeed;
		AttackRange += Other.AttackRange;
		return *this;
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) 
	{
		uint32 PackedMaxHP = 0;
		uint32 PackedAttack = 0;
		uint32 PackedMovementSpeed = 0;
		uint32 PackedAttackSpeed = 0;
		uint32 PackedAttackRange = 0;

		if (Ar.IsSaving())
		{
			PackedMaxHP			= (uint32)FMath::Clamp(FMath::RoundToInt(MaxHP), 0, 65535);
			PackedAttack		= (uint32)FMath::Clamp(FMath::RoundToInt(Attack), 0, 65535);
			PackedMovementSpeed = (uint32)FMath::Clamp(FMath::RoundToInt(MovementSpeed), 0, 65535);
			PackedAttackSpeed	= (uint32)FMath::Clamp(FMath::RoundToInt(AttackSpeed * 100.f), 0, 65535);	// 소수점 2자리까지 사용하기 때문(예: 1.25)
			PackedAttackRange	= (uint32)FMath::Clamp(FMath::RoundToInt(AttackRange), 0, 65535);
		}
		
													// 스탯 설계 기준, 값 범위에 따라
		Ar.SerializeIntPacked(PackedMaxHP);			// 2byte
		Ar.SerializeIntPacked(PackedAttack);		// 1~2byte
		Ar.SerializeIntPacked(PackedMovementSpeed);	// 2byte
		Ar.SerializeIntPacked(PackedAttackSpeed);	// 1~2byte
		Ar.SerializeIntPacked(PackedAttackRange);	// 1byte

		if (Ar.IsLoading())
		{
			MaxHP = (float)PackedMaxHP;
			Attack = (float)PackedAttack;
			MovementSpeed = (float)PackedMovementSpeed;
			AttackSpeed = (float)PackedAttackSpeed / 100.f;
			AttackRange = (float)PackedAttackRange;
		}

		bOutSuccess = true;
		return true;
	}
};

template<>
struct TStructOpsTypeTraits<FCGBaseStat> : public TStructOpsTypeTraitsBase2<FCGBaseStat>
{
	enum
	{
		WithNetSerializer = true,
	};
};

UCLASS()
class CHRONICLEGATE_API UCGStatData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BaseStat)
	FCGBaseStat Base;
	
	
};
