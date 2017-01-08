// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Object.h"
#include "AtomTypes.h"
#include "AtomImpactEffect.generated.h"

/**
* Represents an impact effect that will be played on a specific material type 
* with AtomImpactEffect.
*/
USTRUCT()
struct FMaterialEffect
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly)
	class USoundBase* Sound = nullptr;

	UPROPERTY(EditDefaultsOnly)
	class UParticleSystem* Particles = nullptr;

	UPROPERTY(EditDefaultsOnly)
	FDecalInfo Decal;
};

/**
* An impact effect that can be used to visually indicate an impact on a specific 
* physical material surface. Used for impacts with instant shot types.
*
* Should never by spawned. Instead, use SpawnEffect on the default object.
*/
UCLASS(Abstract, Blueprintable, NotPlaceable, Config = Game)
class PROJECTATOMVR_API UAtomImpactEffect : public UObject
{
	GENERATED_BODY()

public:

	/**
	* Spawns the impact effect.
	*
	* @param World	The world context.
	* @param Hit	The hit impact to play to effect for.
	*/
	UFUNCTION(BlueprintCallable, Category = ImpactEffect)
	virtual void SpawnEffect(UWorld* World, const FHitResult& Hit) const;

	//-----------------------------------------------------------------
	// UObject Interface 
	//-----------------------------------------------------------------
	virtual void BeginDestroy() override;
	//-----------------------------------------------------------------
	// UObject Interface End
	//-----------------------------------------------------------------

protected:
	/** Effects play for specific physical materials */
	UPROPERTY(EditDefaultsOnly, Category = Impact)
	FMaterialEffect SurfaceEffects[SurfaceType_Max];

private:
	/** Gets the surface effect for the specified physical material */
	const FMaterialEffect& GetEffect(TWeakObjectPtr<UPhysicalMaterial> Material) const;
};
