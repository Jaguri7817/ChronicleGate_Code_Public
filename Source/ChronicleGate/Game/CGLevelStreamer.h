#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Interface/CGInteractInterface.h"
#include "CGLevelStreamer.generated.h"

UCLASS()
class CHRONICLEGATE_API ACGLevelStreamer : public AActor, public ICGInteractInterface
{
	GENERATED_BODY()

public:
    ACGLevelStreamer();


protected:
    // 박스 콜리전 영역, 상호작용
    UPROPERTY(VisibleAnywhere) UBoxComponent* InteractArea;

    virtual void BeginPlay() override;

    // 박스 콜리전 영역 충돌 검사, 상호작용
    UFUNCTION() void OnAreaBegin(UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);
    // 박스 콜리전 영역 충돌 검사 종료될 때, 상호작용
    UFUNCTION() void OnAreaEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // ====== CGInteractInterface 인터페이스 구현 ======
    virtual FText GetInteractPrompt_Implementation() const override;
    virtual bool  CanInteract_Implementation(APawn* InstigatorPawn) const override;
    virtual void  Interact_Implementation(APawn* InstigatorPawn) override;

    
// UI Section
public:
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<class UUserWidget> PressEWidgetClass;

    TObjectPtr<class UUserWidget> PressEInstance;
};
