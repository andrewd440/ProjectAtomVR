// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "EquippableHUDActor.h"
#include "UserWidget.h"
#include "EquippableWidget.h"
#include "EquippableWidgetComponent.h"
#include "AtomLoadout.h"


AEquippableHUDActor::AEquippableHUDActor()
{

}

void AEquippableHUDActor::OnLoadoutChanged(ELoadoutSlotChangeType Type, const FAtomLoadoutSlot& LoadoutSlot)
{
	for (UEquippableWidget* Widget : EquippableWidgets)
	{
		Widget->OnLoadoutChanged(Type, LoadoutSlot);
	}
}

class AAtomEquippable* AEquippableHUDActor::GetEquippable() const
{
	return Equippable.Get();
}

void AEquippableHUDActor::SetEquippable(AAtomEquippable* NewEquippable)
{
	Equippable = NewEquippable;

	if (NewEquippable)
	{
		Equippable->OnEquippedStatusChangedUI.BindUObject(this, &AEquippableHUDActor::OnEquippedStatusChanged);

		UpdateWidgetAttachments();				
	}	
}

void AEquippableHUDActor::PostInitializeComponents()
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

void AEquippableHUDActor::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);

	for (UEquippableWidget* Widget : EquippableWidgets)
	{
		Widget->OnNewEquippable();
	}
}

void AEquippableHUDActor::Destroyed()
{
	EquippableWidgets.Empty();

	Super::Destroyed();
}

void AEquippableHUDActor::UpdateWidgetAttachments()
{
	check(Equippable.IsValid());

	AttachToActor(Equippable.Get(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);

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

			WidgetComponent->AttachToComponent(AttachParent, FAttachmentTransformRules::KeepRelativeTransform, AttachSocket);
		}
		else
		{
			// Attach to item loadout slot
			UAtomLoadout* Loadout = Equippable->GetCharacterOwner()->GetLoadout();
			Loadout->SetItemUIRoot(Equippable.Get(), WidgetComponent);
		}
	}
}

void AEquippableHUDActor::OnEquippedStatusChanged()
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
