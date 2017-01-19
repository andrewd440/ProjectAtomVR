// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomHUDActor.h"
#include "WidgetComponent.h"
#include "UserWidget.h"
#include "WidgetTree.h"
#include "VRHUD.h"


// Sets default values
AAtomHUDActor::AAtomHUDActor()
{
	bNetLoadOnClient = false;
}

class AVRHUD* AAtomHUDActor::GetHUD() const
{
	check(GetOwner() == nullptr || Cast<AVRHUD>(GetOwner()));

	return Cast<AVRHUD>(GetOwner());
}

const TArray<UUserWidget*> AAtomHUDActor::GetWidgets() const
{
	return Widgets;
}

void AAtomHUDActor::PostInitializeComponents()
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
	if (AVRHUD* HUD = GetHUD())
	{
		for (UUserWidget* Widget : Widgets)
		{
			Widget->SetPlayerContext(HUD->GetPlayerController());
		}
	}
}

void AAtomHUDActor::Destroyed()
{
	Widgets.Empty();

	Super::Destroyed();
}
