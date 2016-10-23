// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "EquippableStateFiring.h"

#include "HeroEquippable.h"
#include "Components/InputComponent.h"

void UEquippableStateFiring::OnEnteredState()
{
	Super::OnEnteredState();

	bIsFiring = true;
	StartFireShotTimer();
}

void UEquippableStateFiring::OnReturnedState()
{
	Super::OnReturnedState();

	bIsFiring = true;
	StartFireShotTimer();
}

void UEquippableStateFiring::OnExitedState()
{
	Super::OnExitedState();

	GetFirearm()->StopFiringEffects();
	bIsFiring = false;

	if (FireTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(FireTimer);
	}
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
	const FFirearmStats& FirearmStats = GetFirearm()->GetFirearmStats();

	const float ShotDelay = FMath::Max(0.f, FirearmStats.FireRate - (GetWorld()->GetTimeSeconds() - LastShotTimestamp));
	GetWorld()->GetTimerManager().SetTimer(FireTimer, this, &UEquippableStateFiring::OnFireShot, FirearmStats.FireRate, true, ShotDelay);

	ShotsFired = 0;
}

void UEquippableStateFiring::OnFireShot()
{
	GetFirearm()->FireShot();

	LastShotTimestamp = GetWorld()->GetTimeSeconds();

	++ShotsFired;
	if ((BurstCount > 0 && ShotsFired >= BurstCount) || GetFirearm()->GetRemainingClip() <= 0)
	{
		GetEquippable()->PopState(this);
	}
}
