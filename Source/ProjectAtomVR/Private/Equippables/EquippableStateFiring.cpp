// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "EquippableStateFiring.h"

#include "HeroEquippable.h"
#include "Components/InputComponent.h"
#include "HeroFirearm.h"

void UEquippableStateFiring::OnEnteredState()
{
	Super::OnEnteredState();

	StartFireShotTimer();
	GetEquippable<AHeroFirearm>()->StartFiringSequence();
}

void UEquippableStateFiring::OnExitedState()
{
	Super::OnExitedState();

	GetEquippable<AHeroFirearm>()->StopFiringSequence();

	if (FireTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(FireTimer);
	}
}

void UEquippableStateFiring::OnTriggerReleased()
{
	GetEquippable()->PopState(this);
}

void UEquippableStateFiring::BindStateInputs(UInputComponent* InputComponent)
{
	Super::BindStateInputs(InputComponent);

	// Only care if fully automatic. Otherwise, the state will be popped when
	// the burst finishes.
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

void UEquippableStateFiring::StartFireShotTimer()
{
	const FFirearmStats& FirearmStats = GetEquippable<AHeroFirearm>()->GetFirearmStats();

	const float ShotDelay = FirearmStats.FireRate - (GetWorld()->GetTimeSeconds() - LastShotTimestamp);
	if (ShotDelay > 0.f)
	{
		// Entered firing state faster than firing rate
		OnFalseFire();
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(FireTimer, this, &UEquippableStateFiring::OnFireShot, FirearmStats.FireRate, true, 0.f);
	}	

	ShotsFired = 0;
}

void UEquippableStateFiring::OnFireShot()
{
	AHeroFirearm* const Firearm = GetEquippable<AHeroFirearm>();

	if (!Firearm->CanFire())
	{
		OnFalseFire();
	}
	else
	{
		Firearm->FireShot();

		LastShotTimestamp = GetWorld()->GetTimeSeconds();

		++ShotsFired;
		if (BurstCount > 0 && ShotsFired >= BurstCount)
		{
			GetEquippable()->PopState(this);
		}
	}

}

void UEquippableStateFiring::OnFalseFire()
{
	// #AtomTodo Signal false fire
	AHeroFirearm* const Firearm = GetEquippable<AHeroFirearm>();
	Firearm->FalseFire();
	Firearm->PopState(this);
}
