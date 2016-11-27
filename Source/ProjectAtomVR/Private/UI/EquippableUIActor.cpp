// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "EquippableUIActor.h"

#include "UI/EquippableWidget.h"
#include "UI/EquippableWidgetComponent.h"


AEquippableUIActor::AEquippableUIActor()
{

}

void AEquippableUIActor::OnLoadoutChanged(ELoadoutSlotChangeType Type)
{
	for (UEquippableWidget* Widget : EquippableWidgets)
	{
		Widget->OnLoadoutChanged(Type);
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
		if (auto EquippableWidget = Cast<UEquippableWidget>(WidgetComponent->GetUserWidgetObject()))
		{
			EquippableWidgets.Add(EquippableWidget);
			EquippableWidget->SetOwner(this);
		}

		if (auto Equippable = Cast<AHeroEquippable>(GetOwner()))
		{
			if (auto EquippableWidgetComponent = Cast<UEquippableWidgetComponent>(WidgetComponent))
			{
				USceneComponent* AttachParent = nullptr;
				FName AttachSocket = NAME_None;

				if (EquippableWidgetComponent->GetWidgetType() == EEquippableWidgetType::Item)
				{
					// Attach to item
					AttachParent = Equippable->GetMesh();
					AttachSocket = EquippableWidgetComponent->GetEquippableAttachSocket();
				}
				else
				{
					// Attach to item loadout slot
					UHeroLoadout* Loadout = Equippable->GetHeroOwner()->GetLoadout();
					const FHeroLoadoutSlot* LoadoutSlot = Loadout->GetItemSlot(Equippable);

					if (LoadoutSlot)
					{
						AttachParent = Loadout->GetAttachParent();
						AttachSocket = LoadoutSlot->StorageSocket;
					}					
				}

				if (AttachParent)
				{
					EquippableWidgetComponent->AttachToComponent(AttachParent, FAttachmentTransformRules::KeepRelativeTransform, AttachSocket);
				}
			}	

			Equippable->OnEquippedStatusChangedUI.BindUObject(this, &AEquippableUIActor::OnEquippedStatusChanged);
		}		
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

void AEquippableUIActor::OnEquippedStatusChanged()
{
	check(Cast<AHeroEquippable>(GetOwner()));

	AHeroEquippable* Equippable = static_cast<AHeroEquippable*>(GetOwner());

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
