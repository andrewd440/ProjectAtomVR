// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "EquippableStateActive.h"

#include "HeroEquippable.h"


void UEquippableStateActive::OnStatePushed()
{
	Super::OnStatePushed();

	GetEquippable()->OnEquipped();
}

void UEquippableStateActive::OnStatePopped()
{
	Super::OnStatePopped();

	GetEquippable()->OnUnequipped();
}
