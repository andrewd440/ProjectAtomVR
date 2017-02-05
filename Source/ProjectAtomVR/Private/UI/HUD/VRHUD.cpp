// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "VRHUD.h"

#include "AtomEquippable.h"
#include "AtomLoadout.h"
#include "AtomLoadoutTemplate.h"
#include "EquippableHUDActor.h"
#include "IConsoleManager.h"
#include "Engine/World.h"
#include "AtomFloatingText.h"
#include "AtomCharacter.h"
#include "HMDCameraComponent.h"

DEFINE_LOG_CATEGORY(LogVRHUD);


namespace
{
	static TAutoConsoleVariable<int32> ShowObjectHelpIndicators(TEXT("HUD.ShowObjectHelpIndicators"), 1, TEXT("Toggles if object help indicators are shown."));
}

AVRHUD::AVRHUD()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	bShowHelp = true;
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

	// Destroy all indicators and clear pending ones
	for (auto& ActiveIndicator : ActiveHelpIndicators)
	{
		if (ActiveIndicator.Indicator.IsValid())
		{
			ActiveIndicator.Indicator->Destroy();
			ActiveIndicator.Indicator = nullptr;
		}
	}

	FTimerManager& TimerManager = GetWorldTimerManager();

	for (auto& PendingIndicator : PendingHelpIndicators)
	{
		TimerManager.ClearTimer(PendingIndicator.TimerHandle);
	}

	Super::Destroyed();
}

void AVRHUD::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Update active help with head position
	APawn* Pawn = PlayerController->GetPawn();
	if (Pawn && GEngine->HMDDevice.IsValid() && ActiveHelpIndicators.Num() > 0)
	{
		FVector RoomSpaceHeadLocation;
		FQuat RoomSpaceHeadOrientation;
		GEngine->HMDDevice->GetCurrentOrientationAndPosition( /* Out */ RoomSpaceHeadOrientation, /* Out */ RoomSpaceHeadLocation);
	
		FTransform HeadTransform = FTransform(
			RoomSpaceHeadOrientation,
			RoomSpaceHeadLocation,
			FVector(1.0f));

		HeadTransform = HeadTransform * Pawn->GetTransform(); // Get world transform

		for (int32 i = 0; i < ActiveHelpIndicators.Num();)
		{
			if (ActiveHelpIndicators[i].Indicator.IsValid())
			{
				ActiveHelpIndicators[i].Indicator->Update(HeadTransform.GetLocation());
				++i;
			}
			else
			{
				// Remove any expired indicators
				ActiveHelpIndicators.RemoveAt(i);
			}
		}	
	}
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

void AVRHUD::ShowHelpIndicator(FHelpIndicatorHandle& HelpHandle, const FText& Text, USceneComponent* AttachParent, 
	const FName AttachSocket, const float Lifetime, const float Delay)
{
	HelpHandle.Handle = NextHelpIndicatorHandle++;

	if (Delay > 0)
	{
		const int32 Index = PendingHelpIndicators.Emplace(HelpHandle, Text, AttachParent, AttachSocket, Lifetime);
	 	FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &AVRHUD::CreatePendingHelpIndicator, HelpHandle.Handle);
		GetWorldTimerManager().SetTimer(PendingHelpIndicators[Index].TimerHandle, TimerDelegate, Delay, false);
	}
	else
	{
		CreateActiveHelpIndicator(HelpHandle, Text, AttachParent, AttachSocket, Lifetime);
	}
}

void AVRHUD::CreatePendingHelpIndicator(const uint64 Handle)
{
	const int32 Index = PendingHelpIndicators.IndexOfByPredicate([Handle](auto& Indicator) 
	{ 
		return Indicator.HelpHandle.Handle == Handle; 
	});

	check(Index != INDEX_NONE);

	const FPendingHelpIndicator& Indicator = PendingHelpIndicators[Index];
	CreateActiveHelpIndicator(Indicator.HelpHandle, Indicator.Text, Indicator.AttachParent, Indicator.AttachSocket, Indicator.LifeTime);
	PendingHelpIndicators.RemoveAt(Index);
}

void AVRHUD::CreateActiveHelpIndicator(const FHelpIndicatorHandle& Handle, const FText& Text, USceneComponent* AttachParent, const FName AttachSocket, const float Lifetime)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.ObjectFlags |= RF_Transient;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AAtomFloatingText* HelpIndicator = GetWorld()->SpawnActor<AAtomFloatingText>(SpawnParams);
	HelpIndicator->AttachToComponent(AttachParent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachSocket);
	HelpIndicator->SetText(Text);

	HelpIndicator->SetLifeSpan(Lifetime);

	// Make sure this handle does not already exist
	check(ActiveHelpIndicators.FindByPredicate([Handle](const FActiveHelpIndicator& ActiveIndicator) { return ActiveIndicator.HelpHandle == Handle; }) == nullptr);
	ActiveHelpIndicators.Emplace(HelpIndicator, Handle);
}

void AVRHUD::ClearHelpIndicator(FHelpIndicatorHandle& Handle)
{
	if (Handle.IsValid())
	{
		// First check pending indicators
		const int32 PendingIndex = PendingHelpIndicators.IndexOfByPredicate([Handle](const FPendingHelpIndicator& PendingIndicator)
		{
			return PendingIndicator.HelpHandle == Handle;
		});

		if (PendingIndex != INDEX_NONE)
		{
			FPendingHelpIndicator& PendingIndicator = PendingHelpIndicators[PendingIndex];

			if (PendingIndicator.TimerHandle.IsValid())
			{
				GetWorldTimerManager().ClearTimer(PendingIndicator.TimerHandle);
			}		

			PendingHelpIndicators.RemoveAt(PendingIndex);
		}
		else
		{
			// Not in pending indicators, check active indicators
			const int32 ActiveIndex = ActiveHelpIndicators.IndexOfByPredicate([Handle](const FActiveHelpIndicator& ActiveIndicator)
			{
				return ActiveIndicator.HelpHandle == Handle;
			});

			if (ActiveIndex != INDEX_NONE)
			{
				FActiveHelpIndicator& ActiveIndicator = ActiveHelpIndicators[ActiveIndex];

				if (ActiveIndicator.Indicator.IsValid())
				{
					ActiveIndicator.Indicator->Destroy();
					ActiveHelpIndicators.RemoveAt(ActiveIndex);
				}
			}
		}

		Handle.Reset();
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