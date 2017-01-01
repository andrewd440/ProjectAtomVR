// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "CartridgeAmmoLoader.h"
#include "AtomFirearm.h"

UCartridgeAmmoLoader::UCartridgeAmmoLoader(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	LoadTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("CartridgeReloadTrigger"));		
	LoadTrigger->SetIsReplicated(false);
	LoadTrigger->SetSphereRadius(2.f);
	//LoadTrigger->SetHiddenInGame(false);
	LoadTrigger->bGenerateOverlapEvents = false;
	LoadTrigger->SetCollisionProfileName(AtomCollisionProfiles::HandTrigger);

	LoadTrigger->OnComponentBeginOverlap.AddDynamic(this, &UCartridgeAmmoLoader::OnHandEnteredReloadTrigger);
}

void UCartridgeAmmoLoader::LoadAmmo(UObject* LoadObject)
{
	Super::LoadAmmo(LoadObject);

	check(LoadObject == nullptr || LoadObject->IsA(CartridgeType));

	if (LoadObject != nullptr)
	{
		AAtomEquippable* Cartridge = static_cast<AAtomEquippable*>(LoadObject);		

		Cartridge->SetCanReturnToLoadout(false);
		if (Cartridge->IsEquipped())
		{
			Cartridge->Unequip(GetFirearm()->HasAuthority() ? EEquipType::Normal : EEquipType::Deferred); // On defer unequip when not authority. Let authority unequip all remotes.
		}		

		Cartridge->SetActorHiddenInGame(true);
		Cartridge->SetLifeSpan(1.f); // Give time to replicate unequip
	}

	ensure(AmmoCount < Capacity);
	++AmmoCount;

	if (AmmoCount >= Capacity)
	{
		LoadTrigger->bGenerateOverlapEvents = false;
	}

	OnAmmoCountChanged.ExecuteIfBound();
}

void UCartridgeAmmoLoader::OnEquipped()
{
	Super::OnEquipped();

	if (AmmoCount < Capacity && GetFirearm()->GetHeroOwner()->IsLocallyControlled())
	{
		LoadTrigger->bGenerateOverlapEvents = true;
	}	
}

void UCartridgeAmmoLoader::OnUnequipped()
{
	Super::OnUnequipped();

	LoadTrigger->bGenerateOverlapEvents = false;
}

void UCartridgeAmmoLoader::ConsumeAmmo()
{
	Super::ConsumeAmmo();

	if (AmmoCount < Capacity && GetFirearm()->GetHeroOwner()->IsLocallyControlled())
	{
		LoadTrigger->bGenerateOverlapEvents = true;
	}
}

void UCartridgeAmmoLoader::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	ReplicateAmmoCount(OutLifetimeProps);
}

void UCartridgeAmmoLoader::InitializeLoader()
{
	Super::InitializeLoader();

	if (!GetFirearm()->IsTemplate())
	{
		LoadTrigger->AttachToComponent(GetFirearm()->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform);
	}

	AmmoCount = Capacity;

	OnAmmoCountChanged.ExecuteIfBound();
}

void UCartridgeAmmoLoader::OnHandEnteredReloadTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	check(Cast<AAtomCharacter>(OtherActor) && "HeroBase should be the only response to this trigger.");
	check(GetFirearm()->IsEquipped() && "Overlap events should be unbound when not the HeroFirearm is not equipped.");

	AAtomCharacter* OverlapHero = static_cast<AAtomCharacter*>(OtherActor);
	AAtomFirearm* MyFirearm = GetFirearm();	

	if (OverlapHero == MyFirearm->GetHeroOwner()) // This is our hero
	{
		AAtomEquippable* OtherEquippable = OverlapHero->GetEquippable(!MyFirearm->GetEquippedHand());

		if (OtherEquippable && OtherEquippable->IsA(CartridgeType)) // The opposite hand equippable is the right cartridge type
		{				
			MyFirearm->LoadAmmo(OtherEquippable);
		}
	}
}
