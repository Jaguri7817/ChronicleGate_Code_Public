// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/CGWidgetComponent.h"
#include "CGUserWidget.h"

void UCGWidgetComponent::InitWidget()
{
	Super::InitWidget();

	UCGUserWidget* CGUserWidget = Cast<UCGUserWidget>(GetWidget());
	if (CGUserWidget)
	{
		CGUserWidget->SetOwningActor(GetOwner());
	}
}
