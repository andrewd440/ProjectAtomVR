// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "EquippableStateActive.h"
#include "EquippableStateActiveFirearm.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UEquippableStateActiveFirearm : public UEquippableStateActive
{
	GENERATED_BODY()

protected:
	virtual void OnTriggerPressed();

	/** UEquippable Interface Begin */
protected:
	virtual void BindStateInputs(UInputComponent* InputComponent) override;
	/** UEquippable Interface End */
};