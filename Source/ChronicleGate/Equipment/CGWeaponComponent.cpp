// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/CGWeaponComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"

// Sets default values for this component's properties
UCGWeaponComponent::UCGWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
   
	// 이 컴포넌트가 들고 있을 무기 메쉬
	WeaponMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	if (WeaponMeshComp)
	{
		WeaponMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMeshComp->SetGenerateOverlapEvents(false);
		WeaponMeshComp->SetCanEverAffectNavigation(false);
		WeaponMeshComp->SetIsReplicated(false); // 부모만 복제
	}
    
	// 캐릭터 스킨 이후, 무기 스킨까지 확장할 때 SoftObject 참조로 바꾸기
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SwordRef(TEXT("/Script/Engine.SkeletalMesh'/Game/InfinityBladeWeapons/Weapons/Blade/Swords/Blade_HeroSword10/SK_Blade_HeroSword10.SK_Blade_HeroSword10'"));
	if (SwordRef.Object)
	{
		DefaultMesh = SwordRef.Object;
	}

}


// Called when the game starts
void UCGWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (OwnerChar && WeaponMeshComp)
	{
		WeaponMeshComp->AttachToComponent(OwnerChar->GetMesh(),	FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachSocketName);
	}

	// 여기서 기본 검을 끼워준다
	if (GetOwnerRole() == ROLE_Authority)
	{
		// 만약 아직 CurrentMesh가 안 정해졌으면
		if (!CurrentMesh && DefaultMesh)
		{
			CurrentMesh = DefaultMesh;
		}
	}

	// 서버에 이미 세팅돼 있던 걸 클라에서도 반영
	if (CurrentMesh && WeaponMeshComp)
	{
		WeaponMeshComp->SetSkeletalMesh(CurrentMesh);
	}
}

void UCGWeaponComponent::SetWeaponMesh(USkeletalMesh* NewMesh)
{
	// 스킨 바꾸는 건 서버에서만
	if (GetOwnerRole() < ROLE_Authority)
	{
		return;
	}

	CurrentMesh = NewMesh;
	OnRep_WeaponMesh();
}

void UCGWeaponComponent::OnRep_WeaponMesh()
{
	if (WeaponMeshComp)
	{
		WeaponMeshComp->SetSkeletalMesh(CurrentMesh);
	}
}

void UCGWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCGWeaponComponent, CurrentMesh);
}
