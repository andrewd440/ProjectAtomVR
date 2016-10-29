// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "HeroEquippable.h"
#include "FirearmClip.generated.h"

UCLASS()
class PROJECTATOMVR_API AFirearmClip : public AHeroEquippable
{
	GENERATED_BODY()
	
public:	
	AFirearmClip(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	int32 GetAmmoCount() const;
	void SetAmmoCount(int32 Count);

private:
	int32 AmmoCount = 0;
};
