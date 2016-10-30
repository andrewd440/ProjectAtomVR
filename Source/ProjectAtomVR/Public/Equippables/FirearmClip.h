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

	/** AActor Interface Begin */
	virtual void Tick(float DeltaSeconds) override;
	/** AActor Interface End */
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = FirearmClip, meta = (AllowPrivateAccess = "true"))
	UShapeComponent* ClipLoadTrigger;
};
