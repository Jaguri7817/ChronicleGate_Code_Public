// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CGWeaponComponent.generated.h"




UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CHRONICLEGATE_API UCGWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCGWeaponComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// 무기 스킨(메쉬) 교체
	UFUNCTION(BlueprintCallable)
	void SetWeaponMesh(class USkeletalMesh* NewMesh);
		

protected:
	// 기본으로 끼울 검 (에디터에서 드래그해서 넣어줄 거)
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	USkeletalMesh* DefaultMesh;

	// 현재 선택된 메쉬 (복제됨)
	UPROPERTY(ReplicatedUsing = OnRep_WeaponMesh)
	USkeletalMesh* CurrentMesh;

	UFUNCTION()
	void OnRep_WeaponMesh();

	// 실제로 손에 붙어 있는 컴포넌트
	UPROPERTY()
	class USkeletalMeshComponent* WeaponMeshComp;

	// 붙일 소켓
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName AttachSocketName = TEXT("hand_rSocket");

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	
};
