// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/CGSkinChanger.h"
#include "Components/BoxComponent.h"
#include "Character/CGCharacterPlayer.h"
#include "CGGameMode.h"        // 필요하면
#include "Net/UnrealNetwork.h"
#include "Controller/CGCharacterPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "BlockChain/CGBackendSubsystem.h"
#include "Game/CGGameInstance.h"

// Sets default values
ACGSkinChanger::ACGSkinChanger()
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

    InteractArea->OnComponentBeginOverlap.AddDynamic(this, &ACGSkinChanger::OnAreaBegin);
    InteractArea->OnComponentEndOverlap.AddDynamic(this, &ACGSkinChanger::OnAreaEnd);

    static ConstructorHelpers::FClassFinder<UUserWidget> PressEWidgetRef(TEXT("/Game/HUD/WBP_PressE.WBP_PressE_C"));
    if (PressEWidgetRef.Class)
    {
        PressEWidgetClass = PressEWidgetRef.Class;
    }
}

void ACGSkinChanger::BeginPlay()
{
    Super::BeginPlay();

}

void ACGSkinChanger::OnAreaBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    UE_LOG(LogTemp, Warning, TEXT("[SkinStation] BeginOverlap by %s (HasAuthority=%d)"), *GetNameSafe(OtherActor), HasAuthority());

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
                    UE_LOG(LogTemp, Warning, TEXT("[SkinStation] Created PressEWidgetClass"));
                }
            }
        }
    }
}


void ACGSkinChanger::OnAreaEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    UE_LOG(LogTemp, Warning, TEXT("[SkinStation] EndOverlap by %s (HasAuthority=%d)"), *GetNameSafe(OtherActor), HasAuthority());

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
            
            HidePressEWidget();
        }
    }
}

FText ACGSkinChanger::GetInteractPrompt_Implementation() const
{
    // 화면에 뜨는 E키 안내 문구
    return FText::FromString(TEXT("[E] 캐릭터 스킨 변경"));
}

bool ACGSkinChanger::CanInteract_Implementation(APawn* Inst) const
{
    // 스킨 변경이 불가능한 상황이 있으면 여기서 막기
    // 예: 트래블 중에는 변경 불가라든가…
    // 지금은 간단히 항상 true로 둬도 됨
    const ACGGameMode* GameMode = GetWorld()->GetAuthGameMode<ACGGameMode>();
    if (!GameMode) return true;

    // 트래블 중엔 못 바꾸게
    if (GameMode->IsTraveling()) return false;

    return true;
}

void ACGSkinChanger::Interact_Implementation(APawn* InstigatorPawn)
{

    UE_LOG(LogTemp, Warning, TEXT("[SkinStation & ServerRPC] Interact_Implementation"));

    if (!HasAuthority()) return;

    if (!InstigatorPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SkinStation] InstigatorPawn is null"));
        return;
    }

    // 이 상호작용을 누른 플레이어의 컨트롤러
    AController* Controller = InstigatorPawn->GetController();
    if (!Controller)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SkinStation] No Controller for InstigatorPawn"));
        return;
    }

    if (ACGCharacterPlayerController* PlayerController = Cast<ACGCharacterPlayerController>(Controller))
    {
        // 여기서 클라이언트 RPC 호출
        PlayerController->ClientRPC_OpenSkinChangeUI();
        if (ACGCharacterPlayer* Character = Cast<ACGCharacterPlayer>(InstigatorPawn))
        {
            MulticastRPC_HidePressEWidget_Implementation(Character);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[SkinStation] Controller is not ACGPlayerController"));
    }
}

void ACGSkinChanger::HidePressEWidget()
{
    if (PressEInstance)
    {
        PressEInstance->RemoveFromParent();
        PressEInstance = nullptr;
        UE_LOG(LogTemp, Warning, TEXT("[SkinStation] Deleted PressEWidgetClass"));
    }
}

void ACGSkinChanger::MulticastRPC_HidePressEWidget_Implementation(ACGCharacterPlayer* InteractingPlayer)
{
    if (!InteractingPlayer || !InteractingPlayer->IsLocallyControlled())
    {
        return;
    }
    HidePressEWidget();
}


