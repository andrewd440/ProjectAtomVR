// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomFloatingUI.h"

#include "AtomUISystem.h"
#include "UserWidget.h"
#include "AtomWidgetComponent.h"


AAtomFloatingUI::AAtomFloatingUI()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	WidgetComponent = CreateDefaultSubobject<UAtomWidgetComponent>(TEXT("WidgetComponent"));
	RootComponent = WidgetComponent;

	WidgetComponent->SetTwoSided(false);
	WidgetComponent->SetDrawAtDesiredSize(true);
}

void AAtomFloatingUI::SetWidget(AAtomUISystem* InUISystem, TSubclassOf<UUserWidget> InWidget)
{
	check(InWidget != nullptr);
	WidgetClass = InWidget;
	SetUISystem(InUISystem);

	SetupWidgetComponent();
}

void AAtomFloatingUI::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AAtomFloatingUI::SetupWidgetComponent()
{
	check(WidgetClass != nullptr);

	Widget = CreateWidget<UUserWidget>(GetWorld(), WidgetClass);
	check(Widget != nullptr);

	Widget->SetPlayerContext(GetUISystem()->GetPlayerController());

	WidgetComponent->SetWidget(Widget);
}
