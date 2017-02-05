// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomFloatingUI.h"
#include "WidgetComponent.h"
#include "UserWidget.h"
#include "SWidget.h"


// Sets default values
AAtomFloatingUI::AAtomFloatingUI()
{
	bReplicates = false;
	bNetLoadOnClient = true;

	SetActorEnableCollision(false);

	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
	RootComponent = WidgetComponent;
	WidgetComponent->SetCastShadow(false);
}

void AAtomFloatingUI::SetUMGWidget(TSubclassOf<UUserWidget> Widget)
{
	UMGWidgetClass = Widget;
}

void AAtomFloatingUI::SetSlateWidget(const TSharedRef<SWidget>& Widget)
{
	SlateWidget = Widget;
}

void AAtomFloatingUI::UpdateWidgetComponent()
{
	if (SlateWidget.IsValid())
	{
		WidgetComponent->SetSlateWidget(SlateWidget);
	}
	else if (UMGWidgetClass != nullptr)
	{
		UMGWidget = CreateWidget<UUserWidget>(GetWorld(), UMGWidgetClass);
		WidgetComponent->SetWidget(UMGWidget);
	}
}

void AAtomFloatingUI::Destroyed()
{
	if (WidgetComponent != nullptr)
	{
		WidgetComponent->SetSlateWidget(nullptr);
		WidgetComponent->SetWidget(nullptr);
		WidgetComponent = nullptr;
	}

	SlateWidget = nullptr;

	if (UMGWidget != nullptr)
	{
		UMGWidget->MarkPendingKill();
		UMGWidget = nullptr;
	}

	Super::Destroyed();
}
