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
			CreateLoadoutTriggers(LoadoutTemplateSlots);
		}

		// Weapons will only be spawned by server
		if (Owner->HasAuthority())
		{
			CreateLoadoutWeapons(LoadoutTemplateSlots);
		}
	}
}

bool UHeroLoadout::RequestEquip(UPrimitiveComponent* OverlapComponent, const EHand Hand)
{
	for (FHeroLoadoutSlot& Slot : Loadout)
	{
		if (OverlapComponent->IsOverlappingComponent(Slot.StorageTrigger) && Slot.Item->CanEquip(Hand))
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
		HeroOwner->Unequip(Item);
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

	if (OverlappedSlot && OverlappedSlot->Item)
	{
		EControllerHand ControllerHand = EControllerHand::Left;
		EHand Hand = EHand::Left;

		ensureMsgf(OtherComp == HeroOwner->GetHandMesh<EHand::Right>() || OtherComp == HeroOwner->GetHandMesh<EHand::Left>(), TEXT("Loadout slot triggers should only overlapped hero hand meshes. Check your collision setups."));
		if (OtherComp == HeroOwner->GetHandMesh<EHand::Right>())
		{
			ControllerHand = EControllerHand::Right;
			Hand = EHand::Right;
		}

		// Check if the item can be equipped. If it is already equipped, check if the overlapped hand has the item equipped.
		const AHeroEquippable* CurrentlyEquipped = HeroOwner->GetEquippable(Hand);

		if ((CurrentlyEquipped == nullptr && OverlappedSlot->Item->CanEquip(Hand)) || 
			(OverlappedSlot->Item->IsEquipped() && CurrentlyEquipped == OverlappedSlot->Item))
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

void UHeroLoadout::CreateLoadoutWeapons(const TArray<FHeroLoadoutTemplateSlot>& LoadoutTemplateSlots)
{
	if (UWorld* const World = GetWorld())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Instigator = HeroOwner;
		SpawnParams.Owner = HeroOwner;

		for (int32 i = 0; i < LoadoutTemplateSlots.Num(); ++i)
		{
			const FHeroLoadoutTemplateSlot& Slot = LoadoutTemplateSlots[i];

			if (Slot.ItemClass)
			{
				AHeroEquippable* const Equippable = GetWorld()->SpawnActor<AHeroEquippable>(Slot.ItemClass, FTransform::Identity, SpawnParams);
				Equippable->AttachToComponent(HeroOwner->GetBodyMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, Slot.StorageSocket);

				Loadout[i].Item = Equippable;
			}
			else
			{
				UE_LOG(LogHeroLoadout, Warning, TEXT("FHeroLoadoutTemplateSlot item was null. No item will be created from this slot."));
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
