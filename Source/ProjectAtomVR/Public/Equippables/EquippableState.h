// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Object.h"
#include "EquippableState.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class PROJECTATOMVR_API UEquippableState : public UObject
{
	GENERATED_BODY()
	
public:
	UEquippableState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** 
	 * Called when this state has been entered for the owning Equippable.
	 * This can happen when this state is added to the top of the stack, or the stack
	 * has returned to this state.
	 */
	virtual void OnEnteredState();

	/**
	* Called when this state is exiting for the owning Equippable.
	* This happens after this state is popped off the state stack, or a new
	* state has been pushed onto the stack.
	*/
	virtual void OnExitedState();

	/**
	* Called when this state has been pushed onto the top of the stack.
	*/
	virtual void OnStatePushed();

	/**
	* Called when this state has been popped onto from the stack.
	*/
	virtual void OnStatePopped();

	template <typename EquippableType = AHeroEquippable>
	EquippableType* GetEquippable() const
	{
		check(EquippableType::StaticClass()->IsChildOf(Equippable->StaticClass()) && "Owner Equippable is not of the requested type.");
		return static_cast<EquippableType*>(Equippable);
	}

	/** UObject Interface Begin */
	virtual class UWorld* GetWorld() const override final;
	virtual bool IsSupportedForNetworking() const override final;
	/** UObject Interface End */

protected:
	/**
	 * Called when the state is entered or returned to in order to allow input binding.
	 * These bindings are automatically removed when the state is no longer on the top
	 * of the stack.
	 **/
	virtual void BindStateInputs(UInputComponent* InputComponent);

private:
	class AHeroEquippable* Equippable = nullptr;
};
