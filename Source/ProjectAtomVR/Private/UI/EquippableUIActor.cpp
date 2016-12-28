// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "EquippableUIActor.h"

#include "UI/EquippableWidget.h"
#include "UI/EquippableWidgetComponent.h"
#include "AtomLoadout.h"
#include "UserWidget.h"


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

	const TArray<UUserWidget*>& MyWidgets = GetWidgets();
	for (UUserWidget* Widget : MyWidgets)
	{
		if (UEquippableWidget* EquippableWidget = Cast<UEquippableWidget>(Widget))
		{
			EquippableWidgets.Add(EquippableWidget);
			EquippableWidget->SetOwner(this);
		}	
	}

	if (Equippable.IsValid())
	{
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

			AttachParent = Equippable->GetHeroOwner()->GetBodyMesh();
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
