// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "EquippableState.h"
#include "EquippableStateFiring.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UEquippableStateFiring : public UEquippableState
{
	GENERATED_BODY()

protected:
	/**
	* Handler for trigger released input.	
	*/
	void OnTriggerReleased();

	void StartFireShotTimer();

	/**
	* Either performs a dry fire or shot fire based on the state of the weapon. If burst count is expired, this state will be 
	* popped. Only used on Authority and Autonomous.
	* @returns
	*/
	void OnFireShot();

	/**
	* Performs a dry fire and pops this state. Should only be used with autonomous and authority. This will toggle the dry fire 
	* indicated to replicate to remotes.
	*/
	void OnDryFire();

	/**
	* Used for simulating shots on simulated proxies. Starts the firing timer to use OnFireSimulatedShot.
	*/
	void StartSimulatedShotTimer();

	/**
	* Fires shots and increments ShotsFired. Once at BurstCount, this state is popped if it is the top state.
	* Only to be used for simulated proxies.
	*/
	void OnFireSimulatedShot();

private:
	UFUNCTION()
	void OnRep_TotalShotCounter();

	UFUNCTION()
	void OnRep_DryFireNotify();

	/** UEquippableState Interface Begin */
public:
	virtual void OnEnteredState() override;
	virtual void OnExitedState() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> & OutLifetimeProps) const override;	

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
	UPROPERTY(ReplicatedUsing = OnRep_TotalShotCounter)
	int32 ShotsFired = 0;

	uint8 RemoteShotCounter = 0;

	// Used on server to notify clients of firing event
	UPROPERTY(ReplicatedUsing = OnRep_TotalShotCounter)
	uint8 ServerShotCounter = 0;

	UPROPERTY(ReplicatedUsing = OnRep_DryFireNotify)
	uint32 bDryFireNotify : 1; // Switch back and forward on server when a dry fire occurs
};