// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomUISystem.h"
#include "AtomLoadout.h"
#include "AtomLoadoutTemplate.h"

#include "UI/EquippableUIActor.h"
#include "AtomEquippable.h"

UAtomUISystem::UAtomUISystem()
{

}

void UAtomUISystem::SetOwner(AAtomPlayerController* InOwner)
{
	Owner = InOwner;
}

AAtomPlayerController* UAtomUISystem::GetOwner() const
{
	return Owner;
}

AAtomCharacter* UAtomUISystem::GetHero() const
{
	return Owner->GetHero();
}

void UAtomUISystem::SpawnHeroUI()
{
	check(HeroUI.Equippables.Num() == 0);

	UAtomLoadout* Loadout = GetHero()->GetLoadout();
	const auto& TemplateSlots = Loadout->GetLoadoutTemplate().GetDefaultObject()->GetLoadoutSlots();
	auto& LoadoutSlots = Loadout->GetLoadoutSlots();

	HeroUI.Equippables.SetNum(LoadoutSlots.Num());

	for (int32 i = 0; i < TemplateSlots.Num(); ++i)
	{
		FAtomLoadoutSlot& LoadoutSlot = LoadoutSlots[i];
		const FAtomLoadoutTemplateSlot& TemplateSlot = TemplateSlots[i];

		// Create the UI only if the item is created. If not, (i.e. not replicated yet) wait until the item changed
		// event tells us it is there.
		if (LoadoutSlot.Item != nullptr)
		{
			const TSubclassOf<AEquippableUIActor> EquippableUIClass = TemplateSlot.ItemClass->GetDefaultObject<AAtomEquippable>()->GetUIActor();

			FActorSpawnParameters SpawnParams;
			SpawnParams.ObjectFlags |= RF_Transient;
			SpawnParams.Owner = LoadoutSlot.Item;
			AEquippableUIActor* EquippableUI = GetWorld()->SpawnActor<AEquippableUIActor>(EquippableUIClass, SpawnParams);

			HeroUI.Equippables[i] = EquippableUI;
		}

		LoadoutSlot.OnSlotChanged.BindUObject(this, &UAtomUISystem::OnLoadoutSlotChanged, i);
	}
}

void UAtomUISystem::DestroyHeroUI()
{
	for (AEquippableUIActor* Equippable : HeroUI.Equippables)
	{
		if (Equippable)
		{
			Equippable->Destroy();
		}		
	}

	HeroUI.Equippables.Empty();
}

class UWorld* UAtomUISystem::GetWorld() const
{
	check(Owner);
	return Owner->GetWorld();
}

void UAtomUISystem::OnLoadoutSlotChanged(ELoadoutSlotChangeType Change, int32 LoadoutIndex)
{
	UAtomLoadout* Loadout = GetHero()->GetLoadout();
	const auto& LoadoutSlots = Loadout->GetLoadoutSlots();

	AEquippableUIActor*& UIActor = HeroUI.Equippables[LoadoutIndex];

	if ((Change & ELoadoutSlotChangeType::Item) == ELoadoutSlotChangeType::Item)
	{
		AAtomEquippable* NewItem = LoadoutSlots[LoadoutIndex].Item;

		if (NewItem != nullptr)
		{
			if (UIActor == nullptr)
			{
				// Not created yet, make it now
				const auto& TemplateSlots = Loadout->GetLoadoutTemplate().GetDefaultObject()->GetLoadoutSlots();
				const auto EquippableUIClass = TemplateSlots[LoadoutIndex].ItemClass->GetDefaultObject<AAtomEquippable>()->GetUIActor();

				FActorSpawnParameters SpawnParams;
				SpawnParams.ObjectFlags |= RF_Transient;
				SpawnParams.Owner = NewItem;
				UIActor = GetWorld()->SpawnActor<AEquippableUIActor>(EquippableUIClass, SpawnParams);
			}
			else
			{
				// Just update the owner
				UIActor->SetOwner(NewItem);
			}
		}
		else if (UIActor != nullptr)
		{
			// No equippable owner, so destroy
			UIActor->Destroy();
			UIActor = nullptr;
		}
	}

	Change &= ~ELoadoutSlotChangeType::Item; // Item flag is processed, remove it a move on.

	if (Change != ELoadoutSlotChangeType::None && UIActor != nullptr)
	{
		UIActor->OnLoadoutChanged(Change, LoadoutSlots[LoadoutIndex]);
	}
}