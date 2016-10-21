// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "EquippableStateInactive.h"

#include "Equippables/HeroEquippable.h"


void UEquippableStateInactive::OnReturnedState()
{
	Super::OnReturnedState();

	GetEquippable()->OnUnequipped();
}
