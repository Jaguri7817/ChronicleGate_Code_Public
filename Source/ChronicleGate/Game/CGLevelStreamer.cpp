
#include "Game/CGLevelStreamer.h"
#include "Net/UnrealNetwork.h"
#include "CGStageStateBase.h"
#include "Character/CGCharacterPlayer.h"
#include "Game/CGStageStreamerSystem.h"
#include "Blueprint/UserWidget.h"



ACGLevelStreamer::ACGLevelStreamer()
{
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
	InteractArea->OnComponentBeginOverlap.AddDynamic(this, &ACGLevelStreamer::OnAreaBegin);
	InteractArea->OnComponentEndOverlap.AddDynamic(this, &ACGLevelStreamer::OnAreaEnd);

	static ConstructorHelpers::FClassFinder<UUserWidget> PressEWidgetRef(TEXT("/Game/HUD/WBP_PressE.WBP_PressE_C"));
	if (PressEWidgetRef.Class)
	{
		PressEWidgetClass = PressEWidgetRef.Class;
	}
}

void ACGLevelStreamer::BeginPlay()
{
	Super::BeginPlay();
	//CurrentStagePath = InitialStagePath; // "/Game/Maps/Stage1"
}

void ACGLevelStreamer::OnAreaBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,	const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("[Streamer] BeginOverlap by %s (HasAuthority=%d)"), *GetNameSafe(OtherActor), HasAuthority());

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
					UE_LOG(LogTemp, Warning, TEXT("[Streamer] Created PressEWidgetClass"));
				}
			}
		}			
	}
}

void ACGLevelStreamer::OnAreaEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("[Streamer] EndOverlap by %s (HasAuthority=%d)"), *GetNameSafe(OtherActor), HasAuthority());

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
				UE_LOG(LogTemp, Warning, TEXT("[Streamer] Deleted PressEWidgetClass"));
			}
		}
	}
}

// ================================================================================================================
// 인터페이스 구현부
// ================================================================================================================

FText ACGLevelStreamer::GetInteractPrompt_Implementation() const
{
	return FText::FromString(TEXT("[E] 다음 스테이지로 이동"));
}

bool ACGLevelStreamer::CanInteract_Implementation(APawn* /*Inst*/) const
{
	const UCGStageStreamerSystem* StreamerSubsystem = GetWorld()->GetSubsystem<UCGStageStreamerSystem>();
	if (!StreamerSubsystem)
	{	
		UE_LOG(LogTemp, Log, TEXT("[LevelStreamer] StreamerSubsystem is NULL"));
		return false;
	}
	if (StreamerSubsystem->IsStageStreaming())
	{
		UE_LOG(LogTemp, Log, TEXT("[LevelStreamer] Stage Streaming is TRUE"));
		return false;
	}
	if (const ACGStageStateBase* StageState = GetWorld()->GetGameState<ACGStageStateBase>())
	{
		UE_LOG(LogTemp, Log, TEXT("[LevelStreamer] IsStageCleared? : %d"), StageState->IsStageCleared());
		return StageState->IsStageCleared();
	}
	UE_LOG(LogTemp, Log, TEXT("[LevelStreamer] Can't Interact But Why?"));
	return false;
}

void ACGLevelStreamer::Interact_Implementation(APawn* InstigatorPawn/*Inst*/)
{
	UE_LOG(LogTemp, Warning, TEXT("[Streamer & ServerRPC] Interact_Implementation"));

	// 1. 권한 확인
	if (!HasAuthority()) return;

	// 2. GameState와 Subsystem 접근
	// StageState는 클리어 여부 확인용 / StreamerSubsystem은 다음 스테이지 이동용
	ACGStageStateBase* StageState = GetWorld()->GetGameState<ACGStageStateBase>();
	UCGStageStreamerSystem* StreamerSubsystem = GetWorld()->GetSubsystem<UCGStageStreamerSystem>();

	// 필수 객체 누락 확인
	if (!StageState || !StreamerSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[Streamer] GameState or StreamerSubsystem is missing."));
		return;
	}

	// 3. 상호작용 조건 확인 (기존 로직 유지)
	if (!StageState->IsStageCleared())
	{
		UE_LOG(LogTemp, Error, TEXT("[Streamer] Not cleared yet"));
		return;
	}

	// 4. 서브시스템에 스트리밍 요청
	// Subsystem 함수가 Interactor 정보를 받아 다음 스테이지로 텔레포트 시작
	StreamerSubsystem->SetLastInteractor(InstigatorPawn);
	StreamerSubsystem->HandleLevelShown();
}

