// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Equippables/EquippableState.h"
#include "HeroFirearm.h"

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
	AHeroFirearm* GetFirearm() const;
	virtual void StartFireShotTimer();
	virtual void OnFireShot();

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
	/** UObject Interface End */

protected:
	// Shots fired when trigger is pressed. Fully automatic if 0.
	UPROPERTY(EditDefaultsOnly, Category = FiringState)
	int32 BurstCount;

	UPROPERTY(ReplicatedUsing = OnRep_IsFiring)
	uint32 bIsFiring : 1;

	// Timer used to invoke shots at firearm firerate
	FTimerHandle FireTimer;

	// Timestamp of the last time a show was fired.
	float LastShotTimestamp = 0.f;

	// Number of shots fired since entering the state.
	int32 ShotsFired = 0;
};

FORCEINLINE AHeroFirearm* UEquippableStateFiring::GetFirearm() const
{
	check(Cast<AHeroFirearm>(GetEquippable()));
	return static_cast<AHeroFirearm*>(GetEquippable());
}
