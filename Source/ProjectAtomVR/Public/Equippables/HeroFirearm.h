// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "HeroEquippable.h"
#include "HeroFirearm.generated.h"

class UEquippableState;

UCLASS(Abstract)
class PROJECTATOMVR_API AHeroFirearm : public AHeroEquippable
{
	GENERATED_BODY()
	
public:	
	AHeroFirearm(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void Tick( float DeltaSeconds ) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	FName MuzzleSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	TSubclassOf<UEquippableState> FiringStateTemplate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	TSubclassOf<UEquippableState> ChargingStateTemplate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	TSubclassOf<UEquippableState> ReloadingStateTemplate;
};
