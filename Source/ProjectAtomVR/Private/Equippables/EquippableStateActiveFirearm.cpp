// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "EquippableStateActiveFirearm.h"

#include "HeroFirearm.h"
#include "Components/InputComponent.h"


void UEquippableStateActiveFirearm::OnTriggerPressed()
{
	AHeroFirearm* Firearm = GetEquippable<AHeroFirearm>();
	Firearm->PushState(Firearm->GetFiringState());	
}

void UEquippableStateActiveFirearm::OnEjectClip()
{
	AHeroFirearm* Firearm = GetEquippable<AHeroFirearm>();

	if (Firearm->GetMagazine())
	{
		Firearm->EjectMagazine();
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

	if (Firearm->GetMagazine() == nullptr)
	{
		Firearm->PushState(Firearm->GetReloadingState());
	}
}

void UEquippableStateActiveFirearm::OnStatePushed()
{
	Super::OnStatePushed();

	OnClipMagazineHandle = GetEquippable<AHeroFirearm>()->OnMagazineChanged.AddUObject(this, &UEquippableStateActiveFirearm::OnMagazineAttachmentChanged);
}

void UEquippableStateActiveFirearm::OnStatePopped()
{
	if (OnClipMagazineHandle.IsValid())
	{
		GetEquippable<AHeroFirearm>()->OnMagazineChanged.Remove(OnClipMagazineHandle);
	}

	Super::OnStatePopped();
}

void UEquippableStateActiveFirearm::OnMagazineAttachmentChanged()
{
	AHeroFirearm* Firearm = GetEquippable<AHeroFirearm>();
	if (Firearm->GetMagazine() != nullptr)
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