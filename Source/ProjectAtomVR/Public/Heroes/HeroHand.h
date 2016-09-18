// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "HeroHand.generated.h"

class AHeroBase;

//-----------------------------------------------------------------
// Represents the "hands" of a hero.
//-----------------------------------------------------------------
UCLASS(Blueprintable, Abstract, NotPlaceable)
class PROJECTATOMVR_API AHeroHand : public AActor
{
	GENERATED_BODY()
	
public:
	enum class EHandedness
	{
		Right,
		Left
	};

	enum class EDominance
	{
		Dominate,
		NonDominate
	};

public:	
	AHeroHand(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void SetHandDominance(EHandedness Handedness, EDominance Dominance);

	/** AActor Interface Begin */

	/** UActor Interface End */
	
private:
	/** Motion controller owned by this hero controller. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = HeroController, meta = (AllowPrivateAccess = "true"))
	UMotionControllerComponent* MotionController;

	/** Mesh for the hero right hand (will be reflected for left hand). Usually represents the hero's hand. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = HeroController, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* DefaultMesh;

public:
	FORCEINLINE USkeletalMeshComponent* GetDefaultMesh() const { return DefaultMesh; }
	FORCEINLINE UMotionControllerComponent* GetMotionController() const { return MotionController; }
};
