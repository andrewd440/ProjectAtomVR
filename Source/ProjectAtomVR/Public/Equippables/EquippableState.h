// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Object.h"
#include "EquippableState.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced, Within=HeroEquippable)
class PROJECTATOMVR_API UEquippableState : public UObject
{
	GENERATED_BODY()
	
public:
	UEquippableState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** 
	 * Called when this state has been entered for the owning Equippable.
	 */
	virtual void OnEnteredState();

	/**
	* Called when this state has been returned to from another state.
	* For example, if this state pushes another state onto the Equippable, then that state
	* is popped.
	*/
	virtual void OnReturnedState();

	/**
	* Called when the owning Equippable is being unequipped.
	*/
	virtual void OnUnequip();

protected:
	AHeroEquippable* GetEquippable() const;

private:
	class AHeroEquippable* Equippable = nullptr;
};


FORCEINLINE AHeroEquippable* UEquippableState::GetEquippable() const { return Equippable; }