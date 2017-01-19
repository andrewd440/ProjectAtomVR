// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "VRHUD.h"

#include "AtomEquippable.h"
#include "AtomLoadout.h"
#include "AtomLoadoutTemplate.h"
#include "EquippableHUDActor.h"

DEFINE_LOG_CATEGORY(LogVRHUD);

AVRHUD::AVRHUD()
{

}

AAtomPlayerController* AVRHUD::GetPlayerController() const
{
	return PlayerController;
}

void AVRHUD::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);

	PlayerController = Cast<AAtomPlayerController>(NewOwner);
	ensure(IsActorBeingDestroyed() || PlayerController != nullptr);
}

void AVRHUD::Destroyed()
{
	DestroyLoadoutActors(GetCharacter());

	Super::Destroyed();
}

AAtomCharacter* AVRHUD::GetCharacter() const
{
	return PlayerController ? PlayerController->GetCharacter() : nullptr;
}

void AVRHUD::OnCharacterChanged(AAtomCharacter* OldCharacter)
{
	if (OldCharacter != GetCharacter())
	{
		DestroyLoadoutActors(OldCharacter);

		if (GetCharacter())
		{
			SpawnLoadoutActors();
		}			
	}
}

void AVRHUD::SpawnLoadoutActors()
{
	UE_LOG(LogVRHUD, Verbose, TEXT("%s::SpawnLoadoutActors()"), *GetClass()->GetName());

	check(LoadoutActors.Num() == 0);

	UAtomLoadout* Loadout = GetCharacter()->GetLoadout();
	const auto& TemplateSlots = Loadout->GetTemplateSlots();
	auto& LoadoutSlots = Loadout->GetLoadoutSlots();

	LoadoutActors.SetNum(LoadoutSlots.Num());

	for (int32 i = 0; i < TemplateSlots.Num(); ++i)
	{
		FAtomLoadoutSlot& LoadoutSlot = LoadoutSlots[i];
		const FAtomLoadoutTemplateSlot& TemplateSlot = TemplateSlots[i];
		const auto EquippableUIClass = TemplateSlot.ItemClass->GetDefaultObject<AAtomEquippable>()->GetHUDActor();

		// Create the UI only if the item is created. If not, (i.e. not replicated yet) wait until the item changed
		// event tells us it is there.
		if (LoadoutSlot.Item && EquippableUIClass)
		{		
			auto LoadoutActor = GetWorld()->SpawnActorDeferred<AEquippableHUDActor>(EquippableUIClass, FTransform::Identity, this);
			LoadoutActor->SetFlags(RF_Transient);
			LoadoutActor->SetEquippable(LoadoutSlot.Item);

			LoadoutActor->FinishSpawning(FTransform::Identity, true);

			LoadoutActors[i] = LoadoutActor;
		}

		LoadoutSlot.OnSlotChanged.AddUObject(this, &AVRHUD::OnLoadoutSlotChanged, i);
	}
}

void AVRHUD::DestroyLoadoutActors(AAtomCharacter* OldCharacter)
{
	UE_LOG(LogVRHUD, Verbose, TEXT("%s::DestroyLoadoutActors()"), *GetClass()->GetName());

	for (auto* LoadoutActor : LoadoutActors)
	{
		if (LoadoutActor)
		{
			LoadoutActor->Destroy();
		}		
	}

	LoadoutActors.Empty();

	// Unbind any slot change events
	if (OldCharacter)
	{
		TArray<FAtomLoadoutSlot>& LoadoutSlots = OldCharacter->GetLoadout()->GetLoadoutSlots();

		for (FAtomLoadoutSlot& Slot : LoadoutSlots)
		{
			Slot.OnSlotChanged.RemoveAll(this);
		}
	}
}

void AVRHUD::OnLoadoutSlotChanged(ELoadoutSlotChangeType Change, int32 LoadoutIndex)
{
	UAtomLoadout* Loadout = GetCharacter()->GetLoadout();
	const auto& LoadoutSlots = Loadout->GetLoadoutSlots();

	AEquippableHUDActor*& HUDActor = LoadoutActors[LoadoutIndex];

	if ((Change & ELoadoutSlotChangeType::Item) == ELoadoutSlotChangeType::Item)
	{
		AAtomEquippable* NewItem = LoadoutSlots[LoadoutIndex].Item;

		if (NewItem != nullptr)
		{
			if (HUDActor == nullptr)
			{
				// Not created yet, make it now

				const auto& TemplateSlots = Loadout->GetTemplateSlots();
				const auto EquippableUIClass = NewItem->GetHUDActor();

				if (EquippableUIClass)
				{
					HUDActor = GetWorld()->SpawnActorDeferred<AEquippableHUDActor>(EquippableUIClass, FTransform::Identity, this);
					HUDActor->SetFlags(RF_Transient);
					HUDActor->SetEquippable(NewItem);

					HUDActor->FinishSpawning(FTransform::Identity, true);
				}				
			}
			else
			{
				// Just update the owner
				HUDActor->SetEquippable(NewItem);
			}
		}
		else if (HUDActor != nullptr)
		{
			// No equippable owner, so destroy
			HUDActor->Destroy();
			HUDActor = nullptr;
		}
	}

	Change &= ~ELoadoutSlotChangeType::Item; // Item flag is processed, remove it and move on.

	if (Change != ELoadoutSlotChangeType::None && HUDActor != nullptr)
	{
		HUDActor->OnLoadoutChanged(Change, LoadoutSlots[LoadoutIndex]);
	}
}