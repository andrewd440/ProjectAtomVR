// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AmmoLoader.h"

#include "HeroFirearm.h"

UAmmoLoader::UAmmoLoader(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	Firearm = Cast<AHeroFirearm>(GetOuter());
	ensure(Firearm || (GetFlags() & RF_ArchetypeObject) == RF_ArchetypeObject);
}

void UAmmoLoader::InitializeLoader()
{

}

void UAmmoLoader::SetupInputComponent(class UInputComponent* InputComponent)
{

}

void UAmmoLoader::OnEquipped()
{

}

void UAmmoLoader::OnUnequipped()
{

}

void UAmmoLoader::ConsumeAmmo()
{
	--AmmoCount;
}

void UAmmoLoader::LoadAmmo(UObject* LoadObject)
{

}

bool UAmmoLoader::DiscardAmmo()
{
	return false;
}

bool UAmmoLoader::IsSupportedForNetworking() const
{
	return true;
}

class UWorld* UAmmoLoader::GetWorld() const
{
	return GetFirearm()->GetWorld();
}

UWorld* UAmmoLoader::GetTickableGameObjectWorld() const
{
	return GetWorld();
}

void UAmmoLoader::Tick(float DeltaTime)
{
	
}

TStatId UAmmoLoader::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UAmmoLoader, STATGROUP_Tickables);
}

bool UAmmoLoader::IsTickable() const
{
	return GetFirearm() != nullptr && 
			GetFirearm()->IsEquipped();
}
