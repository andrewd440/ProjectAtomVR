// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Movement/AtomCharacterMovementType.h"
#include "LocomotionMovementType.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Hero), meta = (BlueprintSpawnableComponent))
class PROJECTATOMVR_API ULocomotionMovementType : public UAtomCharacterMovementType
{
	GENERATED_BODY()
	
public:
	ULocomotionMovementType();

	/** UHeroMovementType Interface Begin */
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	/** UHeroMovementType Interface End */

protected:

	/** Callbacks for player input */
	virtual void OnMoveForward(float Value);
	virtual void OnMoveRight(float Value);

	void OnGripPressed();
	void OnGripReleased();

private:
	uint32 bIsGripPressed : 1;
};
