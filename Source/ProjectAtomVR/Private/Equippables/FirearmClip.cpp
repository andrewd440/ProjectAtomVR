// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "FirearmClip.h"

AFirearmClip::AFirearmClip(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UStaticMeshComponent>(AHeroEquippable::MeshComponentName))
{

}

int32 AFirearmClip::GetAmmoCount() const
{
	return AmmoCount;
}

void AFirearmClip::SetAmmoCount(int32 Count)
{
	AmmoCount = Count;
}
