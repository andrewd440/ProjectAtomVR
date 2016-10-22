// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "EquippableState.h"

#include "HeroEquippable.h"
#include "Components/InputComponent.h"

UEquippableState::UEquippableState(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	Equippable = Cast<AHeroEquippable>(GetOuter());
	ensure(Equippable || (GetFlags() & RF_ArchetypeObject) == RF_ArchetypeObject);
}

void UEquippableState::OnEnteredState()
{
	if (Equippable->GetHeroOwner()->IsLocallyControlled())
	{
		UInputComponent* const InputComponent = Equippable->InputComponent;
		check(InputComponent && "InputComponent should always be valid on locally controlled Equippables.");
		BindStateInputs(InputComponent);
	}
}

void UEquippableState::OnExitedState()
{
	if (Equippable->GetHeroOwner()->IsLocallyControlled())
	{
		// Remove all action bindings placed by this object.
		UInputComponent* const InputComponent = Equippable->InputComponent;
		check(InputComponent && "InputComponent should always be valid on locally controlled Equippables.");

		for (int i = 0; i < InputComponent->GetNumActionBindings(); ++i)
		{
			FInputActionBinding& InputBinding = InputComponent->GetActionBinding(i);
			if (InputBinding.ActionDelegate.IsBoundToObject(this))
			{
				InputComponent->RemoveActionBinding(i);
			}
		}
	}
}

void UEquippableState::OnReturnedState()
{
	if (Equippable->GetHeroOwner()->IsLocallyControlled())
	{
		UInputComponent* const InputComponent = Equippable->InputComponent;
		check(InputComponent && "InputComponent should always be valid on locally controlled Equippables.");
		BindStateInputs(InputComponent);
	}
}

void UEquippableState::BindStateInputs(UInputComponent* InputComponent)
{

}
