// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "EquippableStateActiveFirearm.h"

#include "AtomFirearm.h"
#include "Components/InputComponent.h"


void UEquippableStateActiveFirearm::OnTriggerPressed()
{
	AAtomFirearm* Firearm = GetEquippable<AAtomFirearm>();
	Firearm->PushState(Firearm->GetFiringState());	
}

void UEquippableStateActiveFirearm::BindStateInputs(UInputComponent* InputComponent)
{
	Super::BindStateInputs(InputComponent);

	const EHand Hand = GetEquippable()->GetEquippedHand();
	if (Hand == EHand::Left)
	{
		InputComponent->BindAction(TEXT("TriggerLeft"), IE_Pressed, this, &UEquippableStateActiveFirearm::OnTriggerPressed);
	}
	else
	{
		InputComponent->BindAction(TEXT("TriggerRight"), IE_Pressed, this, &UEquippableStateActiveFirearm::OnTriggerPressed);
	}
}