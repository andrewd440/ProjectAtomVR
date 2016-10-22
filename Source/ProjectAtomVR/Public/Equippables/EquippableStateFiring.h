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

private:
	UFUNCTION()
	void OnRep_IsFiring();

	/** UEquippableState Interface Begin */
public:
	virtual void OnEnteredState() override;
	virtual void OnReturnedState() override;
	virtual void OnExitedState() override;

protected:
	virtual void BindStateInputs(UInputComponent* InputComponent);
	/** UEquippableState Interface End */

	/** UObject Interface Begin */
public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> & OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override;
	/** UObject Interface End */

protected:
	UPROPERTY(EditDefaultsOnly, Category = FiringState)
	float Damage;

	UPROPERTY(EditDefaultsOnly, Category = FiringState)
	float FireRate; // Seconds between shots

	UPROPERTY(EditDefaultsOnly, Category = FiringState)
	int32 BurstCount; // Shots fired when trigger is pressed. Fully automatic if 0.

	UPROPERTY(ReplicatedUsing = OnRep_IsFiring)
	uint32 bIsFiring : 1;
};
