// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "HeroMovementType.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class PROJECTATOMVR_API UHeroMovementType : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UHeroMovementType(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Allows a hero movement type to set up custom input bindings for the owning hero. */
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent);

	/** UActorComponent Interface Begin */
	virtual void PostLoad() override;
	/** UActorComponent Interface End */

private:
	class AHeroBase* Hero; // The owning hero

public:
	class AHeroBase* GetHero() const { return Hero; }
};