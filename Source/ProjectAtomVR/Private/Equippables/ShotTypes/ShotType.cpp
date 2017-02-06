// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "ShotType.h"

#include "AtomFirearm.h"

void UShotType::SimulateShot(const FShotData& ShotData)
{

}

class UWorld* UShotType::GetWorld() const
{
	return GetFirearm()->GetWorld();
}

class AAtomFirearm* UShotType::GetFirearm() const
{
	return CastChecked<AAtomFirearm>(GetOuter());
}
