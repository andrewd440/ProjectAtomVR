// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "EquippableUIActor.h"

#include "UI/EquippableWidget.h"
#include "UI/EquippableWidgetComponent.h"
#include "AtomLoadout.h"
#include "UserWidget.h"
#include "WidgetTree.h"


AEquippableUIActor::AEquippableUIActor()
{

}

void AEquippableUIActor::OnLoadoutChanged(ELoadoutSlotChangeType Type, const FAtomLoadoutSlot& LoadoutSlot)
{
	for (UEquippableWidget* Widget : EquippableWidgets)
	{
		Widget->OnLoadoutChanged(Type, LoadoutSlot);
	}
}

class AAtomEquippable* AEquippableUIActor::GetEquippable() const
{
	return Equippable.Get();
}

void AEquippableUIActor::SetEquippable(AAtomEquippable* NewEquippable)
{
	Equippable = NewEquippable;

	if (NewEquippable)
	{
		Equippable->OnEquippedStatusChangedUI.BindUObject(this, &AEquippableUIActor::OnEquippedStatusChanged);
		UpdateWidgetAttachments();
	}	
}

void AEquippableUIActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	TInlineComponentArray<UWidgetComponent*> WidgetComponents;
	GetComponents<UWidgetComponent>(WidgetComponents);

	for (UWidgetComponent* WidgetComponent : WidgetComponents)
	{
		// Set owner for all equippable user widgets
		UUserWidget* Widget = WidgetComponent->GetUserWidgetObject();

		// Add all EquippableWidgets to internal list
		if (auto EquippableWidget = Cast<UEquippableWidget>(Widget))
		{
			EquippableWidgets.Add(EquippableWidget);
		}
		
		Widget->WidgetTree->ForEachWidget([this](UWidget* TreeWidget) 
		{ 
			if (auto EquippableWidget = Cast<UEquippableWidget>(TreeWidget))
			{
				EquippableWidgets.Add(EquippableWidget);
			}
		});		
	}

	// Setup owner for all equippable widgets.
	for (auto EquippableWidget : EquippableWidgets)
	{
		EquippableWidget->SetOwner(this);
	}

	if (Equippable.IsValid())
	{
		Equippable->OnEquippedStatusChangedUI.BindUObject(this, &AEquippableUIActor::OnEquippedStatusChanged);
		UpdateWidgetAttachments();
	}
}

void AEquippableUIActor::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);

	for (UEquippableWidget* Widget : EquippableWidgets)
	{
		Widget->OnNewEquippable();
	}
}

void AEquippableUIActor::Destroyed()
{
	for (UEquippableWidget* Widget : EquippableWidgets)
	{
		if(Widget->IsValidLowLevel() && !Widget->IsPendingKill())
			Widget->ConditionalBeginDestroy();
	}

	EquippableWidgets.Empty();

	Super::Destroyed();
}

void AEquippableUIActor::UpdateWidgetAttachments()
{
	check(Equippable.IsValid());

	TInlineComponentArray<UEquippableWidgetComponent*> WidgetComponents;
	GetComponents<UEquippableWidgetComponent>(WidgetComponents);

	for (UEquippableWidgetComponent* WidgetComponent : WidgetComponents)
	{
		USceneComponent* AttachParent = nullptr;
		FName AttachSocket = NAME_None;

		if (WidgetComponent->GetWidgetType() == EEquippableWidgetType::Item)
		{
			// Attach to item
			AttachParent = Equippable->GetMesh();
			AttachSocket = WidgetComponent->GetEquippableAttachSocket();
		}
		else
		{
			// Attach to item loadout slot
			const UAtomLoadout* Loadout = Equippable->GetHeroOwner()->GetLoadout();
			const FAtomLoadoutSlot& LoadoutSlot = Loadout->GetItemSlot(Equippable.Get());

			AttachParent = Loadout->GetAttachParent();
			AttachSocket = LoadoutSlot.UISocket;
		}

		if (AttachParent)
		{
			WidgetComponent->AttachToComponent(AttachParent, FAttachmentTransformRules::KeepRelativeTransform, AttachSocket);
		}
	}
}

void AEquippableUIActor::OnEquippedStatusChanged()
{
	check(Equippable.IsValid());

	if (Equippable->IsEquipped())
	{
		for (UEquippableWidget* Widget : EquippableWidgets)
		{
			Widget->OnEquipped();
		}
	}
	else
	{
		for (UEquippableWidget* Widget : EquippableWidgets)
		{
			Widget->OnUnequipped();
		}
	}
}
