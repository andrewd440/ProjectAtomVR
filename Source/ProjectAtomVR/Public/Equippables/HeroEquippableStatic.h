// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Equippables/HeroEquippable.h"
#include "HeroEquippableStatic.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API AHeroEquippableStatic : public AHeroEquippable
{
	GENERATED_BODY()
	
public:
	AHeroEquippableStatic(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
