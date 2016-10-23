// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "ShotType.h"

#include "HeroFirearm.h"

void UShotType::SimulateShot(const FShotData& ShotData)
{

}

class UWorld* UShotType::GetWorld() const
{
	return GetFirearm()->GetWorld();
}

class AHeroFirearm* UShotType::GetFirearm() const
{
	return static_cast<AHeroFirearm*>(GetOuter());
}
