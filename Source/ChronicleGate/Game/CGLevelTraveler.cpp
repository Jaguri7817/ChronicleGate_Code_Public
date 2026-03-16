// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/CGLevelTraveler.h"
#include "Net/UnrealNetwork.h"
#include "Character/CGCharacterPlayer.h"
#include "CGGameMode.h"
#include "Blueprint/UserWidget.h"



// Sets default values
ACGLevelTraveler::ACGLevelTraveler()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;

	// 박스 컴포넌트 - 콜리전 상호작용 영역
	InteractArea = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractArea"));
	RootComponent = InteractArea;
	InteractArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractArea->SetGenerateOverlapEvents(true);
	InteractArea->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractArea->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// 박스 컴포넌트 - 충돌 시, 충돌 종료 후의 이벤트 함수 연결
	InteractArea->OnComponentBeginOverlap.AddDynamic(this, &ACGLevelTraveler::OnAreaBegin);
	InteractArea->OnComponentEndOverlap.AddDynamic(this, &ACGLevelTraveler::OnAreaEnd);

	static ConstructorHelpers::FClassFinder<UUserWidget> PressEWidgetRef(TEXT("/Game/HUD/WBP_PressE.WBP_PressE_C"));
	if (PressEWidgetRef.Class)
	{
		PressEWidgetClass = PressEWidgetRef.Class;
	}
}

// Called when the game starts or when spawned
void ACGLevelTraveler::BeginPlay()
{
	Super::BeginPlay();

}


void ACGLevelTraveler::OnAreaBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("[Traveler] BeginOverlap by %s (HasAuthority=%d)"), *GetNameSafe(OtherActor), HasAuthority());
	if (ACGCharacterPlayer* Character = Cast<ACGCharacterPlayer>(OtherActor))
	{
		// 서버에서는
		if (HasAuthority())
		{
			Character->SetCurrentInteractable_ServerOnly(this);
			Character->ClientRPC_SetCurrentInteractable(this);
		}

		// 클라에서는
		if (Character->IsLocallyControlled())
		{
			APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
			if (!PlayerController || !PlayerController->IsLocalController()) return;
			if (PressEWidgetClass && !PressEInstance)
			{

				PressEInstance = CreateWidget<UUserWidget>(PlayerController, PressEWidgetClass);
				if (PressEInstance)
				{
					PressEInstance->AddToViewport();
					UE_LOG(LogTemp, Warning, TEXT("[Traveler] Created PressEWidgetClass"));
				}
			}
		}
	}
}

void ACGLevelTraveler::OnAreaEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("[Traveler] EndOverlap by %s (HasAuthority=%d)"), *GetNameSafe(OtherActor), HasAuthority());
	
	if (ACGCharacterPlayer* Character = Cast<ACGCharacterPlayer>(OtherActor))
	{
		if (HasAuthority())
		{
			Character->ClearCurrentInteractable_ServerOnly(this);
			Character->ClientRPC_ClearCurrentInteractable(this);
			return;
		}
		if (Character->IsLocallyControlled())
		{
			APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
			if (!PlayerController || !PlayerController->IsLocalController()) return;
			if (PressEInstance)
			{
				PressEInstance->RemoveFromParent();
				PressEInstance = nullptr;
				UE_LOG(LogTemp, Warning, TEXT("[Traveler] Deleted PressEWidgetClass"));
			}
		}
	}
}


FText ACGLevelTraveler::GetInteractPrompt_Implementation() const
{
	return FText::FromString(TEXT("[E] 다음 스테이지로 이동"));
}

bool ACGLevelTraveler::CanInteract_Implementation(APawn* /*Inst*/) const
{
	const ACGGameMode* GameMode = GetWorld()->GetAuthGameMode<ACGGameMode>();
	if (!GameMode) return false;
	if (GameMode->IsTraveling()) return false;
	
	return true;
}

void ACGLevelTraveler::Interact_Implementation(APawn* InstigatorPawn/*Inst*/)
{
	UE_LOG(LogTemp, Warning, TEXT("[Traveler & ServerRPC] Interact_Implementation"));

	// 1. 권한 확인
	if (!HasAuthority()) return;

	// 2. GameMode 접근
	// GameMode는 서버 트래블용
	ACGGameMode* GameMode = GetWorld()->GetAuthGameMode<ACGGameMode>();

	// 필수 객체 누락 확인
	if (!GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("[Traveler] GameMode is missing."));
		return;
	}

	if (!GameMode->IsTraveling())
	{
		// 4. 게임모드에 서버 트래블링 요청
		GameMode->TravelToDungeon();
	}
	
}

