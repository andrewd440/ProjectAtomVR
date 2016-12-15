// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomLoadout.h"

#include "AtomLoadoutTemplate.h"
#include "Equippables/AtomEquippable.h"
#include "Haptics/HapticFeedbackEffect_Curve.h"

DEFINE_LOG_CATEGORY_STATIC(LogHeroLoadout, Log, All);

namespace
{
	static const FColor TriggerBaseColor{ 100, 255, 100, 255 };

	struct FSavedLoadoutSlot
	{
		AAtomEquippable* Item;
		uint32 Count;
	};

	bool operator==(const FSavedLoadoutSlot& SavedSlot, const FAtomLoadoutSlot& Slot)
	{
		return SavedSlot.Item == Slot.Item && SavedSlot.Count == Slot.Count;
	}

	bool operator!=(const FSavedLoadoutSlot& SavedSlot, const FAtomLoadoutSlot& Slot)
	{
		return !(SavedSlot == Slot);
	}

	static TArray<FSavedLoadoutSlot> SavedLoadout;
}

const FAtomLoadoutSlot UAtomLoadout::NullLoadoutSlot;

void UAtomLoadout::CreateLoadoutTriggers(const TArray<FAtomLoadoutTemplateSlot>& LoadoutTemplateSlots)
{
	for (int32 i = 0; i < LoadoutTemplateSlots.Num(); ++i)
	{
		USphereComponent* Trigger = NewObject<USphereComponent>(HeroOwner);
		Trigger->SetIsReplicated(false);

		Trigger->RegisterComponent();

		Trigger->SetSphereRadius(LoadoutTemplateSlots[i].StorageTriggerRadius);
		Trigger->SetCollisionProfileName(AtomCollisionProfiles::HandTrigger);		
		Trigger->bGenerateOverlapEvents = true;
		Trigger->ShapeColor = TriggerBaseColor;
		//Trigger->SetHiddenInGame(false);

		Trigger->OnComponentBeginOverlap.AddDynamic(this, &UAtomLoadout::OnLoadoutTriggerOverlap);		

		Trigger->AttachToComponent(GetAttachParent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, LoadoutTemplateSlots[i].StorageSocket);

		Loadout[i].StorageTrigger = Trigger;
	}
}

void UAtomLoadout::InitializeLoadout(class AAtomCharacter* Owner)
{
	HeroOwner = Owner;

	if (LoadoutTemplate)
	{
		const UAtomLoadoutTemplate* const LoadoutTemplateCDO = LoadoutTemplate->GetDefaultObject<UAtomLoadoutTemplate>();
		const TArray<FAtomLoadoutTemplateSlot>& LoadoutTemplateSlots = LoadoutTemplateCDO->GetLoadoutSlots();

		const int32 LoadoutSlotCount = LoadoutTemplateSlots.Num();

		Loadout.SetNum(LoadoutSlotCount);

		// Assign sockets
		for (int32 i = 0; i < LoadoutSlotCount; ++i)
		{
			Loadout[i].StorageSocket = LoadoutTemplateSlots[i].StorageSocket;
			Loadout[i].UISocket = LoadoutTemplateSlots[i].UISocket;
		}
	}
}

void UAtomLoadout::SpawnLoadout()
{
	check(HeroOwner && "Loadout has not been initialized.");

	if (LoadoutTemplate)
	{
		const UAtomLoadoutTemplate* const LoadoutTemplateCDO = LoadoutTemplate->GetDefaultObject<UAtomLoadoutTemplate>();
		const TArray<FAtomLoadoutTemplateSlot>& LoadoutTemplateSlots = LoadoutTemplateCDO->GetLoadoutSlots();

		if (HeroOwner->IsLocallyControlled())
		{
			UE_LOG(LogHeroLoadout, Log, TEXT("Loadout triggers created on %s for %s"), *HeroOwner->GetController()->GetName(), *HeroOwner->GetName());
			CreateLoadoutTriggers(LoadoutTemplateSlots);
		}

		// Weapons will only be spawned by server
		if (HeroOwner->HasAuthority())
		{
			CreateLoadoutEquippables(LoadoutTemplateSlots);
		}
	}
}

bool UAtomLoadout::RequestEquip(UPrimitiveComponent* OverlapComponent, const EHand Hand)
{
	for (FAtomLoadoutSlot& Slot : Loadout)
	{
		if (Slot.Item && Slot.Item->CanEquip(Hand) &&
			OverlapComponent->IsOverlappingComponent(Slot.StorageTrigger))
		{
			HeroOwner->Equip(Slot.Item, Hand);
			return true;
		}
	}	

	return false;
}

bool UAtomLoadout::RequestUnequip(UPrimitiveComponent* OverlapComponent, AAtomEquippable* Item)
{
	const FAtomLoadoutSlot* Slot = Loadout.FindByPredicate([Item](const FAtomLoadoutSlot& Slot) { return Slot.Item == Item; });

	if (Slot && OverlapComponent->IsOverlappingComponent(Slot->StorageTrigger))
	{
		HeroOwner->Unequip(Item, Item->GetEquippedHand());
		return true;
	}

	return false;
}

const TArray<FAtomLoadoutSlot>& UAtomLoadout::GetLoadoutSlots() const
{
	return Loadout;
}

TArray<FAtomLoadoutSlot>& UAtomLoadout::GetLoadoutSlots()
{
	return Loadout;
}

const TSubclassOf<class UAtomLoadoutTemplate> UAtomLoadout::GetLoadoutTemplate() const
{
	return LoadoutTemplate;
}

const FAtomLoadoutSlot& UAtomLoadout::GetItemSlot(const class AAtomEquippable* Item) const
{
	if (const FAtomLoadoutSlot* FoundSlot = Loadout.FindByPredicate([Item](const FAtomLoadoutSlot& Slot) { return Slot.Item == Item; }))
	{
		return *FoundSlot;
	}
	else
	{
		return NullLoadoutSlot;
	}
}

USceneComponent* UAtomLoadout::GetAttachParent() const
{
	return HeroOwner->GetBodyAttachmentComponent();
}

void UAtomLoadout::PreNetReceive()
{
	Super::PreNetReceive();

	SavedLoadout.SetNum(Loadout.Num(), false);

	for (int i = 0; i < Loadout.Num(); ++i)
	{
		SavedLoadout[i].Item = Loadout[i].Item;
		SavedLoadout[i].Count = Loadout[i].Count;
	}
}

class UWorld* UAtomLoadout::GetWorld() const
{
	return HeroOwner ? HeroOwner->GetWorld() : nullptr;
}

void UAtomLoadout::OnLoadoutTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	FAtomLoadoutSlot* const OverlappedSlot = Loadout.FindByPredicate([OverlappedComponent](const FAtomLoadoutSlot& Slot) { return Slot.StorageTrigger == OverlappedComponent; });

	// Try to get the loadout item
	AAtomEquippable* const OverlappedItem = OverlappedSlot ? OverlappedSlot->Item : nullptr;

	if (OverlappedItem)
	{
		EControllerHand ControllerHand = EControllerHand::Left;
		EHand Hand = EHand::Left;

		ensureMsgf(OtherComp == HeroOwner->GetHandTrigger<EHand::Right>() || OtherComp == HeroOwner->GetHandTrigger<EHand::Left>(), TEXT("Loadout slot triggers should only overlapped hero hand triggers. Check your collision setups."));
		if (OtherComp == HeroOwner->GetHandTrigger<EHand::Right>())
		{
			ControllerHand = EControllerHand::Right;
			Hand = EHand::Right;
		}

		// Check if the item can be equipped. If it is already equipped, check if the overlapped hand has the item equipped.
		const AAtomEquippable* CurrentlyEquipped = HeroOwner->GetEquippable(Hand);

		if ((CurrentlyEquipped == nullptr && OverlappedItem->CanEquip(Hand)) ||
			(OverlappedItem->IsEquipped() && CurrentlyEquipped == OverlappedItem))
		{
			APlayerController* const PC = Cast<APlayerController>(HeroOwner->GetController());
			ensureMsgf(PC != nullptr && PC->IsLocalController(), TEXT("Loadout trigger overlaps should only occur on locally controlled heros."));

			if (PC && TriggerFeedback)
			{
				PC->PlayHapticEffect(TriggerFeedback, ControllerHand);
			}
		}
	}
}

void UAtomLoadout::CreateLoadoutEquippables(const TArray<FAtomLoadoutTemplateSlot>& LoadoutTemplateSlots)
{
	if (UWorld* const World = GetWorld())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Instigator = HeroOwner;
		SpawnParams.Owner = HeroOwner;

		for (int32 i = 0; i < LoadoutTemplateSlots.Num(); ++i)
		{
			const FAtomLoadoutTemplateSlot& TemplateSlot = LoadoutTemplateSlots[i];
			FAtomLoadoutSlot& CurrentSlot = Loadout[i];

			if (TemplateSlot.ItemClass)
			{
				AAtomEquippable* const Equippable = GetWorld()->SpawnActor<AAtomEquippable>(TemplateSlot.ItemClass, FTransform::Identity, SpawnParams);
				Equippable->AttachToComponent(GetAttachParent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TemplateSlot.StorageSocket);
				Equippable->SetLoadoutAttachment(GetAttachParent(), TemplateSlot.StorageSocket);

				CurrentSlot.Item = Equippable;
				CurrentSlot.Count = TemplateSlot.Count;

				Equippable->OnCanReturnToLoadoutChanged.AddUObject(this, &UAtomLoadout::OnReturnToLoadoutChanged, Equippable, i);				

				CurrentSlot.OnSlotChanged.ExecuteIfBound(ELoadoutSlotChangeType::Item | ELoadoutSlotChangeType::Count);
			}
			else
			{
				UE_LOG(LogHeroLoadout, Warning, TEXT("FHeroLoadoutTemplateSlot item was null. No item will be created from this slot."));
			}
		}
	}
}

void UAtomLoadout::OnReturnToLoadoutChanged(AAtomEquippable* Item, int32 LoadoutIndex)
{
	if (!Item->CanReturnToLoadout())
	{
		check(LoadoutIndex < Loadout.Num());

		FAtomLoadoutSlot& Slot = Loadout[LoadoutIndex];

		if (Slot.Item == Item) // Make sure this is the current item
		{
			const UAtomLoadoutTemplate* const LoadoutTemplateCDO = LoadoutTemplate->GetDefaultObject<UAtomLoadoutTemplate>();
			const FAtomLoadoutTemplateSlot& TemplateSlot = LoadoutTemplateCDO->GetLoadoutSlots()[LoadoutIndex];
			
			if (Slot.Count > 0)
			{
				// We have more, spawn one
				--Slot.Count;

				FActorSpawnParameters SpawnParams;
				SpawnParams.Instigator = HeroOwner;
				SpawnParams.Owner = HeroOwner;

				Slot.Item = GetWorld()->SpawnActor<AAtomEquippable>(TemplateSlot.ItemClass, FTransform::Identity, SpawnParams);
				Slot.Item->AttachToComponent(GetAttachParent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TemplateSlot.StorageSocket);

				Slot.Item->OnCanReturnToLoadoutChanged.AddUObject(this, &UAtomLoadout::OnReturnToLoadoutChanged, Slot.Item, LoadoutIndex);		

				Slot.OnSlotChanged.ExecuteIfBound(ELoadoutSlotChangeType::Count | ELoadoutSlotChangeType::Item);
			}
			else
			{
				Slot.Item = nullptr;

				Slot.OnSlotChanged.ExecuteIfBound(ELoadoutSlotChangeType::Item);
			}
		}
	}
}

void UAtomLoadout::OnRep_Loadout()
{
	for (int i = 0; i < Loadout.Num(); ++i)
	{
		ELoadoutSlotChangeType Change = ELoadoutSlotChangeType::None;

		if (SavedLoadout[i].Item != Loadout[i].Item)
		{
			Change |= ELoadoutSlotChangeType::Item;

			AAtomEquippable* Item = Loadout[i].Item;

			// Update local attachment only if not equipped. It may be equipped for late joining remotes.
			if (!Item->IsEquipped())
			{
				Item->AttachToComponent(GetAttachParent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, Loadout[i].StorageSocket);
			}
			
			Item->SetLoadoutAttachment(GetAttachParent() ,Loadout[i].StorageSocket);
		}			

		if (SavedLoadout[i].Count != Loadout[i].Count)
			Change |= ELoadoutSlotChangeType::Count;

		if (Change != ELoadoutSlotChangeType::None)
		{
			Loadout[i].OnSlotChanged.ExecuteIfBound(Change);
		}
	}
}

void UAtomLoadout::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAtomLoadout, Loadout);
}

bool UAtomLoadout::IsSupportedForNetworking() const
{
	return true;
}
