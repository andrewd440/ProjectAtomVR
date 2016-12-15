// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "AtomCharacterMovementType.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class PROJECTATOMVR_API UAtomCharacterMovementType : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UAtomCharacterMovementType(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Allows a hero movement type to set up custom input bindings for the owning hero. */
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent);

	/** UActorComponent Interface Begin */
	virtual void PostLoad() override;
	/** UActorComponent Interface End */

private:
	class AAtomCharacter* Hero; // The owning hero

public:
	class AAtomCharacter* GetHero() const { return Hero; }
};