// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "EquippableState.h"

#include "HeroEquippable.h"

UEquippableState::UEquippableState(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	Equippable = Cast<AHeroEquippable>(GetOuter());
	ensure(Equippable || (GetFlags() & RF_ArchetypeObject) == RF_ArchetypeObject);
}

void UEquippableState::OnEnteredState()
{

}

void UEquippableState::OnReturnedState()
{

}

void UEquippableState::OnUnequip()
{

}