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
	GetEquippable<AHeroFirearm>()->StopFiringSequence();

	if (FireTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(FireTimer);
	}

	Super::OnExitedState();
}

void UEquippableStateFiring::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UEquippableStateFiring, TotalShotCounter, COND_SkipOwner);
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

	// Used to update remotes with shot count. Should always increment.
	++TotalShotCounter;

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
		else if (!Firearm->CanFire())
		{
			// This will be the last shot, so stop the sequence. Don't pop state to allow
			// dry fire.
			Firearm->StopFiringSequence();
		}
	}

}

void UEquippableStateFiring::OnFalseFire()
{
	AHeroFirearm* const Firearm = GetEquippable<AHeroFirearm>();
	Firearm->DryFire();
	Firearm->PopState(this);
}

void UEquippableStateFiring::OnRep_TotalShotCounter()
{
	const int32 ShotDiff = FMath::Abs(TotalShotCounter - RemoteShotCounter); // Abs to allow wrapping
	RemoteShotCounter = TotalShotCounter;

	AHeroFirearm* const Firearm = GetEquippable<AHeroFirearm>();

	if (Firearm->HasActorBegunPlay())
	{
		if (ShotDiff > 0)
		{
			// Push state for burst diff
			BurstCount = ShotDiff;

			if (Firearm->GetCurrentState() != this)
			{
				Firearm->PushState(this);
			}
		}
		else if (Firearm->GetCurrentState() == this)
		{
			// Pop state since we are updated
			Firearm->PopState(this);
		}
	}
}