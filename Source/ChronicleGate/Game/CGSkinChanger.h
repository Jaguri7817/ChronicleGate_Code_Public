// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Interface/CGInteractInterface.h"
#include "CGSkinChanger.generated.h"

UCLASS()
class CHRONICLEGATE_API ACGSkinChanger : public AActor, public ICGInteractInterface
{
	GENERATED_BODY()
	
public:	
	ACGSkinChanger();


protected:
    virtual void BeginPlay() override;

    // 상호작용 범위 박스
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
    TObjectPtr<UBoxComponent> InteractArea;

    // 오버랩 시작/종료
    UFUNCTION()
    void OnAreaBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnAreaEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
    // E키 안내 문구
    virtual FText GetInteractPrompt_Implementation() const override;

    // 상호작용 가능한지
    virtual bool CanInteract_Implementation(APawn* Inst) const override;

    // 실제 상호작용 실행 (서버에서 호출)
    virtual void Interact_Implementation(APawn* InstigatorPawn) override;
	

// UI Section
public:
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<class UUserWidget> PressEWidgetClass;

    TObjectPtr<class UUserWidget> PressEInstance;

    void HidePressEWidget();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastRPC_HidePressEWidget(ACGCharacterPlayer* InteractingPlayer);
};
