// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Heroes/HeroBase.h"
#include "TeleportHero.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API ATeleportHero : public AHeroBase
{
	GENERATED_BODY()
	
public:
	ATeleportHero(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
