// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomUISystem.h"
#include "AtomLoadout.h"
#include "AtomLoadoutTemplate.h"

#include "UI/EquippableUIActor.h"
#include "AtomEquippable.h"
#include "GameModeUISubsystem.h"
#include "LevelUIManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogUISystem, Log, All);

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
	ensure(IsActorBeingDestroyed() || PlayerController != nullptr);
}

void AAtomUISystem::Destroyed()
{
	DestroyCharacterUI();
	DestroyGameModeUI();

	Super::Destroyed();
}

AAtomCharacter* AAtomUISystem::GetCharacter() const
{
	return PlayerController->GetCharacter();
}

void AAtomUISystem::CreateCharacterUI()
{
	UE_LOG(LogUISystem, Log, TEXT("Spawning character UI."));
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

			AEquippableUIActor* EquippableUI = GetWorld()->SpawnActorDeferred<AEquippableUIActor>(EquippableUIClass, FTransform::Identity, this);
			EquippableUI->SetFlags(RF_Transient);
			EquippableUI->SetEquippable(LoadoutSlot.Item);

			EquippableUI->FinishSpawning(FTransform::Identity, true);

			HeroUI.Equippables[i] = EquippableUI;
		}

		LoadoutSlot.OnSlotChanged.BindUObject(this, &AAtomUISystem::OnLoadoutSlotChanged, i);
	}
}

void AAtomUISystem::DestroyCharacterUI()
{
	UE_LOG(LogUISystem, Log, TEXT("Destroying character UI."));
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
	DestroyGameModeUI();

	UE_LOG(LogUISystem, Log, TEXT("Creating GameMode UI."));
	if (GameModeClass->IsChildOf(AAtomBaseGameMode::StaticClass()))
	{
		AAtomBaseGameMode* GameModeCDO = GameModeClass->GetDefaultObject<AAtomBaseGameMode>();

		if (TSubclassOf<UGameModeUISubsystem> GameModeUIClass = GameModeCDO->GetUIClass())
		{
			GameModeUI = NewObject<UGameModeUISubsystem>(this, GameModeUIClass, TEXT("GameModeUISubsystem"), RF_Transient);
			GameModeUI->InitializeSystem(this, GameModeCDO);
		}
	}
}

void AAtomUISystem::DestroyGameModeUI()
{
	if (GameModeUI)
	{
		UE_LOG(LogUISystem, Log, TEXT("Destroying GameMode UI."));

		GameModeUI->Destroy();
		GameModeUI = nullptr;
	}
}

void AAtomUISystem::CreateLevelUI()
{
	LevelUI = nullptr;

	for (FActorIterator It(GetWorld()); It && LevelUI == nullptr; ++It)
	{
		LevelUI = Cast<ALevelUIManager>(*It);
	}

	if (LevelUI != nullptr)
	{
		UE_LOG(LogUISystem, Log, TEXT("Creating Level UI."));
		LevelUI->SpawnLevelUI(this);
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

				UIActor = GetWorld()->SpawnActorDeferred<AEquippableUIActor>(EquippableUIClass, FTransform::Identity, this);
				UIActor->SetFlags(RF_Transient);
				UIActor->SetEquippable(NewItem);
				UIActor->SetUISystem(this);

				UIActor->FinishSpawning(FTransform::Identity, true);
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

	Change &= ~ELoadoutSlotChangeType::Item; // Item flag is processed, remove it and move on.

	if (Change != ELoadoutSlotChangeType::None && UIActor != nullptr)
	{
		UIActor->OnLoadoutChanged(Change, LoadoutSlots[LoadoutIndex]);
	}
}