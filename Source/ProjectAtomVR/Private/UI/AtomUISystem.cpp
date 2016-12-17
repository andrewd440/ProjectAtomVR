// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomUISystem.h"
#include "AtomLoadout.h"
#include "AtomLoadoutTemplate.h"

#include "UI/EquippableUIActor.h"
#include "AtomEquippable.h"
#include "GameModeUISubsystem.h"

AAtomUISystem::AAtomUISystem()
{

}

AAtomPlayerController* AAtomUISystem::GetPlayerController() const
{
	return PlayerController;
}

void AAtomUISystem::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);

	PlayerController = Cast<AAtomPlayerController>(NewOwner);
	ensure(PlayerController);
}

AAtomCharacter* AAtomUISystem::GetCharacter() const
{
	return PlayerController->GetHero();
}

void AAtomUISystem::SpawnCharacterUI()
{
	check(HeroUI.Equippables.Num() == 0);

	UAtomLoadout* Loadout = GetCharacter()->GetLoadout();
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
			SpawnParams.Owner = this;
			AEquippableUIActor* EquippableUI = GetWorld()->SpawnActor<AEquippableUIActor>(EquippableUIClass, SpawnParams);
			EquippableUI->SetEquippable(LoadoutSlot.Item);

			HeroUI.Equippables[i] = EquippableUI;
		}

		LoadoutSlot.OnSlotChanged.BindUObject(this, &AAtomUISystem::OnLoadoutSlotChanged, i);
	}
}

void AAtomUISystem::DestroyCharacterUI()
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

void AAtomUISystem::CreateGameModeUI(TSubclassOf<class AGameModeBase> GameModeClass)
{
	if (GameModeUI != nullptr)
	{
		GameModeUI->ConditionalBeginDestroy();
		GameModeUI = nullptr;
	}

	if (GameModeClass->IsChildOf(AAtomGameMode::StaticClass()))
	{
		AAtomGameMode* GameModeCDO = GameModeClass->GetDefaultObject<AAtomGameMode>();

		if (TSubclassOf<UGameModeUISubsystem> GameModeUIClass = GameModeCDO->GetUIClass())
		{
			GameModeUI = NewObject<UGameModeUISubsystem>(this, GameModeUIClass, TEXT("GameModeUISubsystem"), RF_Transient);
			GameModeUI->InitializeSystem(this, GameModeCDO);
		}
	}
}


void AAtomUISystem::OnLoadoutSlotChanged(ELoadoutSlotChangeType Change, int32 LoadoutIndex)
{
	UAtomLoadout* Loadout = GetCharacter()->GetLoadout();
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
				SpawnParams.Owner = this;
				UIActor = GetWorld()->SpawnActor<AEquippableUIActor>(EquippableUIClass, SpawnParams);
				UIActor->SetEquippable(NewItem);
			}
			else
			{
				// Just update the owner
				UIActor->SetEquippable(NewItem);
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