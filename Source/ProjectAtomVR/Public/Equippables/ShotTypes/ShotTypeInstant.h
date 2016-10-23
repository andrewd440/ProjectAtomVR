// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "ShotType.h"
#include "ShotTypeInstant.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UShotTypeInstant : public UShotType
{
	GENERATED_BODY()

protected:

	/**
	* Plays shot trial effects, if assigned, from the current firearm muzzle to a specified end location.
	*/
	void PlayTrailEffects(const FVector& Start, const FVector& End) const;

	/**
	* Plays impact effects, if assigned, at a specified hit.
	*/
	void PlayImpactEffects(const FHitResult& Hit) const;

	/**
	* Performs a weapon trace from a start location to a end location.
	*/
	FHitResult WeaponTrace(const FVector& Start, const FVector& End) const;

	/** UShotType interface */
public:
	virtual FShotData GetShotData() const override;
	virtual void SimulateShot(const FShotData& ShotData) override;
	virtual void FireShot(const FShotData& ShotData) override;
	/** UShotType interface end */

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = ShotTypeInstant)
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = ShotTypeInstant)
	UParticleSystem* TrailFX = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = ShotTypeInstant)
	float ShotRadius = 0.f; // Radius used for sweep hits
};
