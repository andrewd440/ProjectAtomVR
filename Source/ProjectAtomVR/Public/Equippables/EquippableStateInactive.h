// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Equippables/EquippableState.h"
#include "EquippableStateInactive.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UEquippableStateInactive : public UEquippableState
{
	GENERATED_BODY()
	
public:
	virtual void OnReturnedState() override;
};
