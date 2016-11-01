// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Equippables/EquippableState.h"
#include "EquippableStateFiring.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UEquippableStateFiring : public UEquippableState
{
	GENERATED_BODY()

protected:
	virtual void OnTriggerReleased();
	virtual void StartFireShotTimer();
	virtual void OnFireShot();

	/** UEquippableState Interface Begin */
public:
	virtual void OnEnteredState() override;
	virtual void OnExitedState() override;

protected:
	virtual void BindStateInputs(UInputComponent* InputComponent);
	/** UEquippableState Interface End */

protected:
	// Shots fired when trigger is pressed. Fully automatic if 0.
	UPROPERTY(EditDefaultsOnly, Category = FiringState)
	int32 BurstCount;

	// Timer used to invoke shots at firearm firerate
	FTimerHandle FireTimer;

	// Timestamp of the last time a show was fired.
	float LastShotTimestamp = 0.f;

	// Number of shots fired since entering the state.
	int32 ShotsFired = 0;
};