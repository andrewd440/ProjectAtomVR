// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Equippables/EquippableState.h"
#include "EquippableStateActive.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UEquippableStateActive : public UEquippableState
{
	GENERATED_BODY()
	
public:
	virtual void OnEnteredState() override;
};
