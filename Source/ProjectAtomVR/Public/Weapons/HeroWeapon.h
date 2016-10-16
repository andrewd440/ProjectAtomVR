// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "HeroWeapon.generated.h"

UCLASS(Abstract)
class PROJECTATOMVR_API AHeroWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AHeroWeapon(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void Tick( float DeltaSeconds ) override;

	virtual void Equip(class AHeroBase* EquippingHero);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	FName MuzzleSocket;
};
