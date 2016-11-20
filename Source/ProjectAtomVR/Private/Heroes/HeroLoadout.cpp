// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "HeroLoadout.h"

#include "HeroLoadoutTemplate.h"
#include "HeroBase.h"
#include "Equippables/HeroEquippable.h"
#include "Haptics/HapticFeedbackEffect_Curve.h"

DEFINE_LOG_CATEGORY_STATIC(LogHeroLoadout, Log, All);

namespace
{
	static const FColor TriggerBaseColor{ 100, 255, 100, 255 };
}

void UHeroLoadout::CreateLoadoutTriggers(const TArray<FHeroLoadoutTemplateSlot>& LoadoutTemplateSlots)
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
		Trigger->SetHiddenInGame(false);

		Trigger->OnComponentBeginOverlap.AddDynamic(this, &UHeroLoadout::OnLoadoutTriggerOverlap);		

		Trigger->AttachToComponent(HeroOwner->GetBodyMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, LoadoutTemplateSlots[i].StorageSocket);

		Loadout[i].StorageTrigger = Trigger;
	}
}

void UHeroLoadout::InitializeLoadout(class AHeroBase* Owner)
{
	HeroOwner = Owner;

	if (LoadoutTemplate)
	{
		const UHeroLoadoutTemplate* const LoadoutTemplateCDO = LoadoutTemplate->GetDefaultObject<UHeroLoadoutTemplate>();
		const TArray<FHeroLoadoutTemplateSlot>& LoadoutTemplateSlots = LoadoutTemplateCDO->GetLoadoutSlots();

		const int32 LoadoutSlotCount = LoadoutTemplateSlots.Num();

		Loadout.SetNum(LoadoutSlotCount);

		// Assign sockets
		for (int32 i = 0; i < LoadoutSlotCount; ++i)
		{
			Loadout[i].StorageSocket = LoadoutTemplateSlots[i].StorageSocket;
		}

		// Create triggers on locally controlled. Autonomous or listen server controlling owner. Can't use IsLocallyControlled because
		// the controller may not be replicated on autonomous proxies.
		if (Owner->Role == ENetRole::ROLE_AutonomousProxy || (Owner->HasAuthority() && Owner->IsLocallyControlled()))
		{
			UE_LOG(LogHeroLoadout, Log, TEXT("Loadout triggers created on %s for %s"), *Owner->GetController()->GetName(), *Owner->GetName());
			CreateLoadoutTriggers(LoadoutTemplateSlots);
		}

		// Weapons will only be spawned by server
		if (Owner->HasAuthority())
		{
			CreateLoadoutEquippables(LoadoutTemplateSlots);
		}
	}
}

bool UHeroLoadout::RequestEquip(UPrimitiveComponent* OverlapComponent, const EHand Hand)
{
	for (FHeroLoadoutSlot& Slot : Loadout)
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

bool UHeroLoadout::RequestUnequip(UPrimitiveComponent* OverlapComponent, AHeroEquippable* Item)
{
	const FHeroLoadoutSlot* Slot = Loadout.FindByPredicate([Item](const FHeroLoadoutSlot& Slot) { return Slot.Item == Item; });

	if (Slot && OverlapComponent->IsOverlappingComponent(Slot->StorageTrigger))
	{
		HeroOwner->Unequip(Item, Item->GetEquippedHand());
		return true;
	}

	return false;
}

class UWorld* UHeroLoadout::GetWorld() const
{
	return HeroOwner ? HeroOwner->GetWorld() : nullptr;
}

void UHeroLoadout::OnLoadoutTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	FHeroLoadoutSlot* const OverlappedSlot = Loadout.FindByPredicate([OverlappedComponent](const FHeroLoadoutSlot& Slot) { return Slot.StorageTrigger == OverlappedComponent; });

	// Try to get the loadout item
	AHeroEquippable* const OverlappedItem = OverlappedSlot ? OverlappedSlot->Item : nullptr;

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
		const AHeroEquippable* CurrentlyEquipped = HeroOwner->GetEquippable(Hand);

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

void UHeroLoadout::CreateLoadoutEquippables(const TArray<FHeroLoadoutTemplateSlot>& LoadoutTemplateSlots)
{
	if (UWorld* const World = GetWorld())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Instigator = HeroOwner;
		SpawnParams.Owner = HeroOwner;

		for (int32 i = 0; i < LoadoutTemplateSlots.Num(); ++i)
		{
			const FHeroLoadoutTemplateSlot& TemplateSlot = LoadoutTemplateSlots[i];
			FHeroLoadoutSlot& CurrentSlot = Loadout[i];

			if (TemplateSlot.ItemClass)
			{
				AHeroEquippable* const Equippable = GetWorld()->SpawnActor<AHeroEquippable>(TemplateSlot.ItemClass, FTransform::Identity, SpawnParams);
				Equippable->AttachToComponent(HeroOwner->GetBodyMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TemplateSlot.StorageSocket);
				CurrentSlot.Item = Equippable;
				CurrentSlot.Count = TemplateSlot.Count;

				Equippable->OnCanReturnToLoadoutChanged.AddUObject(this, &UHeroLoadout::OnReturnToLoadoutChanged, Equippable, i);												
			}
			else
			{
				UE_LOG(LogHeroLoadout, Warning, TEXT("FHeroLoadoutTemplateSlot item was null. No item will be created from this slot."));
			}
		}
	}
}

void UHeroLoadout::OnReturnToLoadoutChanged(AHeroEquippable* Item, int32 LoadoutIndex)
{
	if (!Item->CanReturnToLoadout())
	{
		check(LoadoutIndex < Loadout.Num());

		FHeroLoadoutSlot& Slot = Loadout[LoadoutIndex];

		if (Slot.Item == Item) // Make sure this is the current item
		{
			const UHeroLoadoutTemplate* const LoadoutTemplateCDO = LoadoutTemplate->GetDefaultObject<UHeroLoadoutTemplate>();
			const FHeroLoadoutTemplateSlot& TemplateSlot = LoadoutTemplateCDO->GetLoadoutSlots()[LoadoutIndex];
			
			if (Slot.Count > 0)
			{
				// We have more, spawn it!
				--Slot.Count;

				FActorSpawnParameters SpawnParams;
				SpawnParams.Instigator = HeroOwner;
				SpawnParams.Owner = HeroOwner;

				Slot.Item = GetWorld()->SpawnActor<AHeroEquippable>(TemplateSlot.ItemClass, FTransform::Identity, SpawnParams);
				Slot.Item->AttachToComponent(HeroOwner->GetBodyMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TemplateSlot.StorageSocket);

				Slot.Item->OnCanReturnToLoadoutChanged.AddUObject(this, &UHeroLoadout::OnReturnToLoadoutChanged, Slot.Item, LoadoutIndex);		
			}
			else
			{
				Slot.Item = nullptr;
			}
		}
	}
}

void UHeroLoadout::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHeroLoadout, Loadout);
}

bool UHeroLoadout::IsSupportedForNetworking() const
{
	return true;
}
