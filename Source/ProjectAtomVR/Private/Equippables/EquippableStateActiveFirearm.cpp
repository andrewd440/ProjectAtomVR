// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "EquippableStateActiveFirearm.h"

#include "HeroFirearm.h"
#include "Components/InputComponent.h"


void UEquippableStateActiveFirearm::OnTriggerPressed()
{
	AHeroFirearm* Firearm = GetEquippable<AHeroFirearm>();

	if (Firearm->GetRemainingClip() > 0 && Firearm->GetClip() != nullptr)
	{
		Firearm->PushState(Firearm->GetFiringState());
	}
}

void UEquippableStateActiveFirearm::OnEjectClip()
{
	AHeroFirearm* Firearm = GetEquippable<AHeroFirearm>();

	if (Firearm->GetClip())
	{
		Firearm->EjectClip();
	}
}

void UEquippableStateActiveFirearm::BindStateInputs(UInputComponent* InputComponent)
{
	Super::BindStateInputs(InputComponent);

	const EHand Hand = GetEquippable()->GetEquippedHand();
	if (Hand == EHand::Left)
	{
		InputComponent->BindAction(TEXT("TriggerLeft"), IE_Pressed, this, &UEquippableStateActiveFirearm::OnTriggerPressed);
		InputComponent->BindAction(TEXT("EjectClipLeft"), IE_Pressed, this, &UEquippableStateActiveFirearm::OnEjectClip);
	}
	else
	{
		InputComponent->BindAction(TEXT("TriggerRight"), IE_Pressed, this, &UEquippableStateActiveFirearm::OnTriggerPressed);
		InputComponent->BindAction(TEXT("EjectClipRight"), IE_Pressed, this, &UEquippableStateActiveFirearm::OnEjectClip);
	}
}

void UEquippableStateActiveFirearm::OnEnteredState()
{
	Super::OnEnteredState();

	AHeroFirearm* Firearm = GetEquippable<AHeroFirearm>();

	if (Firearm->GetClip() == nullptr)
	{
		Firearm->PushState(Firearm->GetReloadingState());
	}
}

void UEquippableStateActiveFirearm::OnStatePushed()
{
	Super::OnStatePushed();

	AHeroFirearm* Firearm = GetEquippable<AHeroFirearm>();
	OnClipChangedHandle = Firearm->OnClipChanged.AddUObject(this, &UEquippableStateActiveFirearm::OnClipAttachmentChanged);
}

void UEquippableStateActiveFirearm::OnStatePopped()
{
	Super::OnStatePopped();

	if (OnClipChangedHandle.IsValid())
	{
		AHeroFirearm* Firearm = GetEquippable<AHeroFirearm>();
		Firearm->OnClipChanged.Remove(OnClipChangedHandle);
	}
}

void UEquippableStateActiveFirearm::OnClipAttachmentChanged()
{
	AHeroFirearm* Firearm = GetEquippable<AHeroFirearm>();
	if (Firearm->GetClip() != nullptr)
	{
		if (Firearm->GetCurrentState() == Firearm->GetReloadingState())
		{
			Firearm->PopState(Firearm->GetReloadingState());
		}
	}
	else
	{
		if (Firearm->GetCurrentState() != Firearm->GetReloadingState())
		{
			Firearm->PushState(Firearm->GetReloadingState());
		}
	}
}
