// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "EquippableStateFiring.h"

#include "HeroEquippable.h"
#include "Components/InputComponent.h"

void UEquippableStateFiring::OnEnteredState()
{
	Super::OnEnteredState();

	bIsFiring = true;
}

void UEquippableStateFiring::OnReturnedState()
{
	Super::OnReturnedState();

	bIsFiring = true;
}

void UEquippableStateFiring::OnExitedState()
{
	Super::OnExitedState();

	bIsFiring = false;
}

void UEquippableStateFiring::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UEquippableStateFiring, bIsFiring, COND_SkipOwner);
}

bool UEquippableStateFiring::IsSupportedForNetworking() const
{
	return true;
}

void UEquippableStateFiring::OnTriggerReleased()
{
	GetEquippable()->PopState(this);
}

void UEquippableStateFiring::OnRep_IsFiring()
{
	if (bIsFiring)
	{
		if (GetEquippable()->GetCurrentState() != this)
		{
			GetEquippable()->PushState(this);
		}
	}
	else
	{
		if (GetEquippable()->GetCurrentState() == this)
		{
			GetEquippable()->PopState(this);
		}
	}
}

void UEquippableStateFiring::BindStateInputs(UInputComponent* InputComponent)
{
	Super::BindStateInputs(InputComponent);

	if (BurstCount == 0)
	{
		const EHand Hand = GetEquippable()->GetEquippedHand();
		if (Hand == EHand::Left)
		{
			InputComponent->BindAction(TEXT("TriggerLeft"), IE_Released, this, &UEquippableStateFiring::OnTriggerReleased);
		}
		else
		{
			InputComponent->BindAction(TEXT("TriggerRight"), IE_Released, this, &UEquippableStateFiring::OnTriggerReleased);
		}
	}
}
