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

	void LoadInto(class AHeroFirearm* Firearm);
	void EjectFrom(class AHeroFirearm* Firearm);

private:
	UPROPERTY(VisibleAnywhere, Category = FirearmClip, meta = (AllowPrivateAccess))
	UShapeComponent* ClipLoadTrigger;
};
