// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "EquippableStateFiring.h"

#include "AtomEquippable.h"
#include "Components/InputComponent.h"
#include "AtomFirearm.h"

void UEquippableStateFiring::OnEnteredState()
{
	Super::OnEnteredState();

	if (GetEquippable()->GetCharacterOwner()->IsLocallyControlled() || 
		GetEquippable()->HasAuthority())
	{
		StartFireShotTimer();
	}
	else
	{
		StartSimulatedShotTimer();
	}
	
	GetEquippable<AAtomFirearm>()->StartFiringSequence();
}

void UEquippableStateFiring::OnExitedState()
{	
	if (FireTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(FireTimer);
	}

	GetEquippable<AAtomFirearm>()->StopFiringSequence();
	
	Super::OnExitedState();
}

void UEquippableStateFiring::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UEquippableStateFiring, ServerShotCounter, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UEquippableStateFiring, bDryFireNotify, COND_SkipOwner);
}

void UEquippableStateFiring::Deactivate()
{
	if (FireTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(FireTimer);
	}

	GetEquippable<AAtomFirearm>()->StopFiringSequence();

	Super::Deactivate();
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
	const AAtomFirearm* const HeroFirearm = GetEquippable<AAtomFirearm>();
	const FFirearmStats& FirearmStats = HeroFirearm->GetFirearmStats();

	const float ShotDelay = FirearmStats.FireRate - (GetWorld()->GetTimeSeconds() - LastShotTimestamp);
	if (ShotDelay > 0.f || HeroFirearm->IsMuzzleInGeometry())
	{
		// Entered firing state faster than firing rate or muzzle is blocked
		OnDryFire();
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(FireTimer, this, &UEquippableStateFiring::OnFireShot, FirearmStats.FireRate, true, 0.f);
	}	

	ShotsFired = 0;
}

void UEquippableStateFiring::StartSimulatedShotTimer()
{
	const AAtomFirearm* const HeroFirearm = GetEquippable<AAtomFirearm>();
	const FFirearmStats& FirearmStats = HeroFirearm->GetFirearmStats();

	GetWorld()->GetTimerManager().SetTimer(FireTimer, this, &UEquippableStateFiring::OnFireSimulatedShot, FirearmStats.FireRate, true, 0.f);

	ShotsFired = 0;
}

void UEquippableStateFiring::OnFireSimulatedShot()
{
	AAtomFirearm* const Firearm = GetEquippable<AAtomFirearm>();

	UE_LOG(LogFirearm, Verbose, TEXT("OnFireSimulatedShot by %s"), Firearm->HasAuthority() ? TEXT("Authority") : TEXT("Client"));

	Firearm->FireShot();

	LastShotTimestamp = GetWorld()->GetTimeSeconds();

	++ShotsFired;

	// Guard against the shot killing us and dropping the firearm, which pops all states.
	if (Firearm->GetCurrentState() == this)
	{
		if (BurstCount > 0 && ShotsFired >= BurstCount)
		{
			// Slide lock may have replicated before shots and gotten overrided, reactivate if needed.
			if (Firearm->IsSlideLockActive())
			{
				Firearm->ActivateSlideLock();
			}

			GetEquippable()->PopState(this);
		}
	}
}

void UEquippableStateFiring::OnFireShot()
{
	AAtomFirearm* const Firearm = GetEquippable<AAtomFirearm>();

	UE_LOG(LogFirearm, Log, TEXT("OnFireShot by %s"), Firearm->HasAuthority() ? TEXT("Authority") : TEXT("Client"));

	if (!Firearm->CanFire())
	{
		OnDryFire();
	}
	else
	{
		Firearm->FireShot();

		LastShotTimestamp = GetWorld()->GetTimeSeconds();

		++ShotsFired;

		// Used to update remotes with shot count.
		++ServerShotCounter;

		// Guard against the shot killing us and dropping the firearm, which pops all states.
		if (Firearm->GetCurrentState() == this)
		{
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
}

void UEquippableStateFiring::OnDryFire()
{
	UE_LOG(LogFirearm, Log, TEXT("OnFalseFire by %s"), GetEquippable<AAtomFirearm>()->HasAuthority() ? TEXT("Authority") : TEXT("Client"));
	AAtomFirearm* const Firearm = GetEquippable<AAtomFirearm>();
	bDryFireNotify = !bDryFireNotify;
	Firearm->DryFire();
	Firearm->PopState(this);
}

void UEquippableStateFiring::OnRep_TotalShotCounter()
{
	const int32 ShotDiff = FMath::Abs(ServerShotCounter - RemoteShotCounter); // Abs to allow wrapping
	RemoteShotCounter = ServerShotCounter;

	UE_LOG(LogFirearm, Log, TEXT("OnRep_TotalShotCounter with ShotDiff =  %d"), ShotDiff);

	AAtomFirearm* const Firearm = GetEquippable<AAtomFirearm>();

	if (Firearm->HasActorBegunPlay())
	{
		if (ShotDiff > 0)
		{
			LastShotTimestamp = 0.f; // Zero shot stamp to force a shot since we know the server shot

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

void UEquippableStateFiring::OnRep_DryFireNotify()
{
	GetEquippable<AAtomFirearm>()->DryFire();
}
