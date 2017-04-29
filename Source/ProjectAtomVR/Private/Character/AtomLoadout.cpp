// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomLoadout.h"

#include "AtomLoadoutTemplate.h"
#include "Equippables/AtomEquippable.h"
#include "Haptics/HapticFeedbackEffect_Curve.h"
#include "Components/StaticMeshComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogLoadout, Log, All);

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
	check(CharacterOwner->IsLocallyControlled());

	for (int32 i = 0; i < LoadoutTemplateSlots.Num(); ++i)
	{
		USphereComponent* Trigger = NewObject<USphereComponent>(CharacterOwner);
		Trigger->SetIsReplicated(false);

		Trigger->RegisterComponent();

		Trigger->SetSphereRadius(LoadoutTemplateSlots[i].StorageTriggerRadius);
		Trigger->SetCollisionProfileName(AtomCollisionProfiles::HandTrigger);		
		Trigger->bGenerateOverlapEvents = true;
		Trigger->ShapeColor = TriggerBaseColor;
		//Trigger->SetHiddenInGame(false);

		Trigger->OnComponentBeginOverlap.AddDynamic(this, &UAtomLoadout::OnLoadoutTriggerOverlap);		

		Trigger->AttachToComponent(GetAttachParent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, 
			LoadoutTemplateSlots[i].StorageSocket);

		Loadout[i].StorageTrigger = Trigger;
	}
}

void UAtomLoadout::InitializeLoadout(class AAtomCharacter* Owner)
{
	CharacterOwner = Owner;

	if (LoadoutTemplate)
	{
		Loadout.SetNum(GetTemplateSlots().Num());
	}
}

void UAtomLoadout::SpawnLoadout()
{
	check(CharacterOwner && "Loadout has not been initialized.");

	if (LoadoutTemplate)
	{
		const TArray<FAtomLoadoutTemplateSlot>& LoadoutTemplateSlots = GetTemplateSlots();

		if (CharacterOwner->IsLocallyControlled())
		{
			UE_LOG(LogLoadout, Log, TEXT("Loadout triggers created on %s for %s"), 
				*CharacterOwner->GetController()->GetName(), *CharacterOwner->GetName());

			CreateLoadoutTriggers(LoadoutTemplateSlots);
		}

		// Weapons will only be spawned by server
		if (CharacterOwner->HasAuthority())
		{
			CreateLoadoutEquippables(LoadoutTemplateSlots);
		}
	}
}

void UAtomLoadout::DisableLoadout()
{
	if (CharacterOwner == nullptr || !CharacterOwner->IsLocallyControlled())
		return;

	for (auto& Slot : Loadout)
	{
		if (Slot.StorageTrigger)
			Slot.StorageTrigger->Deactivate();
	}
}

void UAtomLoadout::DestroyLoadout()
{
	for (FAtomLoadoutSlot& Slot : Loadout)
	{
		if (Slot.StorageTrigger != nullptr)
		{
			Slot.StorageTrigger->DestroyComponent();
			Slot.StorageTrigger = nullptr;
		}

		if (Slot.Item != nullptr)
		{
			Slot.Item->Destroy();
			Slot.Item = nullptr;
		}

		Slot.OnSlotChanged.Clear();
	}
}

bool UAtomLoadout::RequestEquip(UPrimitiveComponent* OverlapComponent, const EHand Hand)
{
	check(CharacterOwner->IsLocallyControlled());

	for (FAtomLoadoutSlot& Slot : Loadout)
	{
		if (Slot.Item && Slot.Item->CanEquip(Hand) &&
			OverlapComponent->IsOverlappingComponent(Slot.StorageTrigger))
		{
			CharacterOwner->Equip(Slot.Item, Hand);
			return true;
		}
	}	

	return false;
}

bool UAtomLoadout::RequestUnequip(UPrimitiveComponent* OverlapComponent, AAtomEquippable* Item)
{
	check(CharacterOwner->IsLocallyControlled());

	const FAtomLoadoutSlot* Slot = Loadout.FindByPredicate([Item](const FAtomLoadoutSlot& Slot) { return Slot.Item == Item; });

	if (Slot && OverlapComponent->IsOverlappingComponent(Slot->StorageTrigger))
	{
		CharacterOwner->Unequip(Slot->Item, Slot->Item->GetEquippedHand());
		return true;
	}

	return false;
}

void UAtomLoadout::OnCharacterControllerChanged()
{
	if (Loadout.Num() <= 0)
		return;
	
	const auto& TemplateSlots = GetTemplateSlots();

	if (CharacterOwner->IsLocallyControlled())
	{
		// Only if BeginPlay has fired.
		if (CharacterOwner->HasActorBegunPlay())
		{
			UE_LOG(LogLoadout, Log, TEXT("OnCharacterControllerChanged() updating loadout for %s controller."), 
				CharacterOwner->GetController() ? *CharacterOwner->GetController()->GetName() : TEXT("nullptr"));			

			// Create triggers if needed
			if (!Loadout[0].StorageTrigger)
			{
				CreateLoadoutTriggers(TemplateSlots);
			}

			// Update all attachments
			for (int32 i = 0; i < Loadout.Num(); ++i)
			{
				const FAtomLoadoutTemplateSlot& TemplateSlot = TemplateSlots[i];
				FAtomLoadoutSlot& Slot = Loadout[i];

				if (Slot.Item)
				{
					Slot.Item->AttachToComponent(GetAttachParent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, 
						TemplateSlot.StorageSocket);
				}

				UpdateSlotOffset(Slot, TemplateSlot);
			}
		}
	}
	else
	{			
		UE_LOG(LogLoadout, Log, TEXT("OnCharacterControllerChanged() updating loadout for %s controller."), 
			CharacterOwner->GetController() ? *CharacterOwner->GetController()->GetName() : TEXT("nullptr"));

		for (int32 i = 0; i < Loadout.Num(); ++i)
		{
			FAtomLoadoutSlot& Slot = Loadout[i];

			// Any triggers should be deleted
			if (Slot.StorageTrigger)
			{
				Slot.StorageTrigger->DestroyComponent();
				Slot.StorageTrigger = nullptr;
			}				

			if (Slot.Item)
			{
				Slot.Item->AttachToComponent(GetAttachParent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, 
					TemplateSlots[i].StorageSocket);
			}
		}
	}
	
}

void UAtomLoadout::SetLoadoutOffset(float Offset)
{
	LoadoutSlotOffset = Offset;

	UpdateAllSlotOffsets();
}

bool UAtomLoadout::IsInLoadout(const AAtomEquippable* Item) const
{
	return GetItemIndex(Item) != INDEX_NONE;
}

void UAtomLoadout::ReturnToLoadout(const AAtomEquippable* Item)
{
	const int32 Index = GetItemIndex(Item);

	check(Index != INDEX_NONE);

	const auto& TemplateSlot = GetTemplateSlots()[Index];
	const auto& Slot = Loadout[Index];

	Slot.Item->AttachToComponent(GetAttachParent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		TemplateSlot.StorageSocket);

	if (CharacterOwner->IsLocallyControlled())
	{
		UpdateSlotOffset(Slot, TemplateSlot);
	}
}

void UAtomLoadout::DiscardFromLoadout(const AAtomEquippable* Item)
{
	const int32 Index = GetItemIndex(Item);
	check(Index != INDEX_NONE);
	check(Loadout[Index].Item == Item);

	FAtomLoadoutSlot& Slot = Loadout[Index];

	// Discards only happen on server
	if (!CharacterOwner->HasAuthority())
	{
		// Only indicate that the item is no longer in the loadout locally
		Slot.Item = nullptr;
	}
	else if (Slot.Count > 1)
	{
		const FAtomLoadoutTemplateSlot& TemplateSlot = GetTemplateSlots()[Index];

		// We have more, spawn one
		--Slot.Count;

		FActorSpawnParameters SpawnParams;
		SpawnParams.Instigator = CharacterOwner;
		SpawnParams.Owner = CharacterOwner;

		Slot.Item = GetWorld()->SpawnActor<AAtomEquippable>(TemplateSlot.ItemClass, FTransform::Identity, SpawnParams);
		Slot.Item->AttachToComponent(GetAttachParent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TemplateSlot.StorageSocket);
		
		if (CharacterOwner->IsLocallyControlled())
		{
			// Offset if locally controlled
			const UStaticMeshComponent* BodyMesh = CharacterOwner->GetBodyMesh();
			FVector ItemSocketOffset = BodyMesh->GetSocketTransform(TemplateSlot.StorageSocket, RTS_Component).GetLocation();
			ItemSocketOffset = ItemSocketOffset.GetSafeNormal2D() * LoadoutSlotOffset;
			Slot.Item->SetActorRelativeLocation(ItemSocketOffset);
		}		

		Slot.OnSlotChanged.Broadcast(ELoadoutSlotChangeType::Count | ELoadoutSlotChangeType::Item);
	}
	else
	{
		Slot.Item = nullptr;

		Slot.OnSlotChanged.Broadcast(ELoadoutSlotChangeType::Item);
	}
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
	const FAtomLoadoutSlot* FoundSlot = Loadout.FindByPredicate([Item](const FAtomLoadoutSlot& Slot)
	{
		return Slot.Item == Item;
	});

	if (FoundSlot)
	{
		return *FoundSlot;
	}
	else
	{
		return NullLoadoutSlot;
	}
}

int32 UAtomLoadout::GetItemIndex(const AAtomEquippable* Item) const
{
	return Loadout.IndexOfByPredicate([Item](const FAtomLoadoutSlot& Slot) { return Slot.Item == Item; });
}

void UAtomLoadout::SetItemUIRoot(const AAtomEquippable* Item, USceneComponent* UIRoot)
{
	const int32 Index = GetItemIndex(Item);

	check(Index != INDEX_NONE);

	FAtomLoadoutSlot& Slot = Loadout[Index];
	const FAtomLoadoutTemplateSlot& TemplateSlot = GetTemplateSlots()[Index];

	Slot.UIRoot = UIRoot;

	// Set offset and update attachment
	UStaticMeshComponent* BodyMesh = CharacterOwner->GetBodyMesh();
	
	UIRoot->AttachToComponent(BodyMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TemplateSlot.UISocket);

	const FTransform SocketTransform = BodyMesh->GetSocketTransform(TemplateSlot.UISocket, RTS_Component);
	FVector SocketOffset = SocketTransform.GetLocation().GetSafeNormal2D() * LoadoutSlotOffset;
	SocketOffset = SocketTransform.GetRotation().UnrotateVector(SocketOffset);

	UIRoot->SetRelativeLocation(SocketOffset);
}

USceneComponent* UAtomLoadout::GetAttachParent() const
{
	return CharacterOwner->GetBodyAttachmentComponent();
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
	return CharacterOwner ? CharacterOwner->GetWorld() : nullptr;
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

		ensureMsgf(OtherComp == CharacterOwner->GetHandTrigger<EHand::Right>() || OtherComp == CharacterOwner->GetHandTrigger<EHand::Left>(), TEXT("Loadout slot triggers should only overlapped hero hand triggers. Check your collision setups."));
		if (OtherComp == CharacterOwner->GetHandTrigger<EHand::Right>())
		{
			ControllerHand = EControllerHand::Right;
			Hand = EHand::Right;
		}

		// Check if the item can be equipped. If it is already equipped, check if the overlapped hand has the item equipped.
		const AAtomEquippable* CurrentlyEquipped = CharacterOwner->GetEquippable(Hand);

		if ((CurrentlyEquipped == nullptr && OverlappedItem->CanEquip(Hand)) ||
			(OverlappedItem->IsEquipped() && CurrentlyEquipped == OverlappedItem))
		{
			APlayerController* const PC = Cast<APlayerController>(CharacterOwner->GetController());
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
		SpawnParams.Instigator = CharacterOwner;
		SpawnParams.Owner = CharacterOwner;

		for (int32 i = 0; i < LoadoutTemplateSlots.Num(); ++i)
		{
			const FAtomLoadoutTemplateSlot& TemplateSlot = LoadoutTemplateSlots[i];
			FAtomLoadoutSlot& CurrentSlot = Loadout[i];

			if (TemplateSlot.ItemClass)
			{
				AAtomEquippable* const Equippable = GetWorld()->SpawnActor<AAtomEquippable>(TemplateSlot.ItemClass, FTransform::Identity, SpawnParams);
				Equippable->AttachToComponent(GetAttachParent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TemplateSlot.StorageSocket);			

				CurrentSlot.Item = Equippable;
				CurrentSlot.Count = TemplateSlot.Count;			

				CurrentSlot.OnSlotChanged.Broadcast(ELoadoutSlotChangeType::Item | ELoadoutSlotChangeType::Count);
			}
			else
			{
				UE_LOG(LogLoadout, Warning, 
					TEXT("FHeroLoadoutTemplateSlot item was null. No item will be created from this slot."));
			}
		}

		if (CharacterOwner->IsLocallyControlled())
		{
			UpdateAllSlotOffsets();
		}		
	}
}

void UAtomLoadout::UpdateAllSlotOffsets()
{
	const auto& TemplateSlots = GetTemplateSlots();
	for (int32 i = 0; i < Loadout.Num(); ++i)
	{
		UpdateSlotOffset(Loadout[i], TemplateSlots[i]);
	}
}

void UAtomLoadout::UpdateSlotOffset(const FAtomLoadoutSlot& Slot, const FAtomLoadoutTemplateSlot& TemplateSlot)
{
	// Only happens on local controllers
	UStaticMeshComponent* BodyMesh = CharacterOwner->GetBodyMesh();

	{
		const FTransform SocketTransform = BodyMesh->GetSocketTransform(TemplateSlot.StorageSocket, RTS_Component);
		FVector SocketOffset = SocketTransform.GetLocation().GetSafeNormal2D() * LoadoutSlotOffset;
		SocketOffset = SocketTransform.GetRotation().UnrotateVector(SocketOffset);

		if (Slot.Item && !Slot.Item->IsEquipped())
		{
			Slot.Item->SetActorRelativeLocation(SocketOffset);
		}

		if (Slot.StorageTrigger)
		{
			Slot.StorageTrigger->SetRelativeLocation(SocketOffset);
		}		
	}

	if (Slot.UIRoot.IsValid())
	{
		const FTransform SocketTransform = BodyMesh->GetSocketTransform(TemplateSlot.UISocket, RTS_Component);
		FVector SocketOffset = SocketTransform.GetLocation().GetSafeNormal2D() * LoadoutSlotOffset;
		SocketOffset = SocketTransform.GetRotation().UnrotateVector(SocketOffset);

		Slot.UIRoot->SetRelativeLocation(SocketOffset);
	}
}

const TArray<FAtomLoadoutTemplateSlot>& UAtomLoadout::GetTemplateSlots() const
{
	const UAtomLoadoutTemplate* const LoadoutTemplateCDO = LoadoutTemplate->GetDefaultObject<UAtomLoadoutTemplate>();
	return LoadoutTemplateCDO->GetLoadoutSlots();
}

void UAtomLoadout::OnRep_Loadout()
{
	for (int i = 0; i < Loadout.Num(); ++i)
	{
		ELoadoutSlotChangeType Change = ELoadoutSlotChangeType::None;

		FAtomLoadoutSlot& Slot = Loadout[i];

		if (SavedLoadout[i].Item != Slot.Item)
		{
			Change |= ELoadoutSlotChangeType::Item;

			const FAtomLoadoutTemplateSlot& TemplateSlot = GetTemplateSlots()[i];

			// Update local attachment only if not equipped. It may be equipped for late joining remotes.
			if (Slot.Item && !Slot.Item->IsEquipped())
			{
				Slot.Item->AttachToComponent(GetAttachParent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale,
					TemplateSlot.StorageSocket);				
			}			

			if (CharacterOwner->IsLocallyControlled())
			{
				UpdateSlotOffset(Slot, TemplateSlot);
			}
		}			

		if (SavedLoadout[i].Count != Slot.Count)
			Change |= ELoadoutSlotChangeType::Count;

		if (Change != ELoadoutSlotChangeType::None)
		{
			Slot.OnSlotChanged.Broadcast(Change);
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
