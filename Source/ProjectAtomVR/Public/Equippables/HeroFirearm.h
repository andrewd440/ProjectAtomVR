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

	UEquippableState* GetFiringState() const;
	UEquippableState* GetChargingState() const;
	UEquippableState* GetReloadingState() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	FName MuzzleSocket;

	UPROPERTY(Instanced, EditDefaultsOnly, BlueprintReadWrite, Category = States)
	UEquippableState* FiringState;

	UPROPERTY(Instanced, EditDefaultsOnly, BlueprintReadWrite, Category = States)
	UEquippableState* ChargingState;

	UPROPERTY(Instanced, EditDefaultsOnly, BlueprintReadWrite, Category = States)
	UEquippableState* ReloadingState;
};

FORCEINLINE UEquippableState* AHeroFirearm::GetFiringState() const { return FiringState; }
FORCEINLINE UEquippableState* AHeroFirearm::GetChargingState() const { return ChargingState; }
FORCEINLINE UEquippableState* AHeroFirearm::GetReloadingState() const { return ReloadingState; }
