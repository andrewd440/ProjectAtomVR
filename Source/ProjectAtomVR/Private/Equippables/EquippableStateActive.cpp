// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "EquippableStateActive.h"

#include "HeroEquippable.h"


void UEquippableStateActive::OnEnteredState()
{
	Super::OnEnteredState();

	GetEquippable()->OnEquipped();
}
