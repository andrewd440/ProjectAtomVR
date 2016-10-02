// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Heroes/Movement/HeroMovementType.h"
#include "LocomotionMovementType.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Hero), meta = (BlueprintSpawnableComponent))
class PROJECTATOMVR_API ULocomotionMovementType : public UHeroMovementType
{
	GENERATED_BODY()
	
public:

	/** UHeroMovementType Interface Begin */
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	/** UHeroMovementType Interface End */

protected:

	/** Callbacks for player input */
	virtual void OnMoveForward(float Value);
	virtual void OnMoveRight(float Value);
};