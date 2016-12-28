// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomUIActor.h"
#include "AtomUISystem.h"
#include "WidgetComponent.h"
#include "UserWidget.h"
#include "WidgetTree.h"


// Sets default values
AAtomUIActor::AAtomUIActor()
{
	bNetLoadOnClient = false;
}

class AAtomUISystem* AAtomUIActor::GetUISystem() const
{
	return UISystem;
}

void AAtomUIActor::SetUISystem(AAtomUISystem* InUISystem)
{
	UISystem = InUISystem;
}

const TArray<UUserWidget*> AAtomUIActor::GetWidgets() const
{
	return Widgets;
}

void AAtomUIActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Collect all widgets
	TInlineComponentArray<UWidgetComponent*> WidgetComponents;
	GetComponents<UWidgetComponent>(WidgetComponents);

	for (UWidgetComponent* WidgetComponent : WidgetComponents)
	{
		// Set owner for all equippable user widgets
		UUserWidget* Widget = WidgetComponent->GetUserWidgetObject();

		// Add all widgets to internal list
		if (Widget != nullptr)
		{
			Widgets.Add(Widget);

			Widget->WidgetTree->ForEachWidget([this](UWidget* TreeWidget)
			{
				if (UUserWidget* UserWidget = Cast<UUserWidget>(TreeWidget))
				{
					Widgets.Add(UserWidget);
				}				
			});
		}
	}

	// Set player context for all widgets
	if (UISystem != nullptr)
	{
		for (UUserWidget* Widget : Widgets)
		{
			Widget->SetPlayerContext(UISystem->GetPlayerController());
		}
	}
}

void AAtomUIActor::Destroyed()
{
	Widgets.Empty();

	Super::Destroyed();
}
