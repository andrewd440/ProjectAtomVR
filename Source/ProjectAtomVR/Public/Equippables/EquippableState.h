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
	* Called when this state is exiting for the owning Equippable.
	* This happens when the state is popped off the state stack, or a new
	* state has been pushed onto the stack.
	*/
	virtual void OnExitedState();

	/**
	* Called when this state has been returned to from another state.
	* For example, if this state pushes another state onto the Equippable, then that state
	* is popped.
	*/
	virtual void OnReturnedState();

protected:
	/**
	 * Called when the state is entered or returned to in order to allow input binding.
	 * These bindings are automatically removed when the state is no longer on the top
	 * of the stack.
	 **/
	virtual void BindStateInputs(UInputComponent* InputComponent);

	AHeroEquippable* GetEquippable() const;

private:
	class AHeroEquippable* Equippable = nullptr;
};


FORCEINLINE AHeroEquippable* UEquippableState::GetEquippable() const { return Equippable; }