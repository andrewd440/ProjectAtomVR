// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "EquippableUIActor.h"

#include "UI/EquippableWidget.h"
#include "UI/EquippableWidgetComponent.h"


void AEquippableUIActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	TInlineComponentArray<UEquippableWidgetComponent*> WidgetComponents;
	GetComponents<UEquippableWidgetComponent>(WidgetComponents);

	for (UEquippableWidgetComponent* WidgetComponent : WidgetComponents)
	{
		if (UEquippableWidget* EquippableWidget = Cast<UEquippableWidget>(WidgetComponent->GetUserWidgetObject()))
		{
			EquippableWidgets.Add(EquippableWidget);
			EquippableWidget->SetOwner(this);
		}

		if (AHeroEquippable* Equippable = Cast<AHeroEquippable>(GetOwner()))
		{
			WidgetComponent->AttachToComponent(Equippable->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, WidgetComponent->GetEquippableAttachSocket());
		}		
	}
}
