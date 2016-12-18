// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Components/WidgetComponent.h"
#include "AtomWidgetComponent.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UAtomWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()
	
public:
	void SetDrawAtDesiredSize(bool bValue);
};
