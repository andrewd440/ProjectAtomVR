// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "HeroEquippable.h"
#include "ShotTypes/ShotType.h"
#include "HeroFirearm.generated.h"

class UEquippableState;

USTRUCT()
struct FFirearmStats
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, Category = Stats)
	float Damage;

	UPROPERTY(EditDefaultsOnly, Category = Stats)
	float FireRate; // Seconds between shots

	UPROPERTY(EditDefaultsOnly, Category = Stats)
	uint32 MaxAmmo;

	UPROPERTY(EditDefaultsOnly, Category = Stats)
	uint32 ClipSize;
};

UCLASS(Abstract)
class PROJECTATOMVR_API AHeroFirearm : public AHeroEquippable
{
	GENERATED_BODY()
	
public:	
	AHeroFirearm(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void Tick( float DeltaSeconds ) override;

	/** Get the location of the firearm muzzle in world space. */
	FVector GetMuzzleLocation() const;

	/** Get the rotation of the firearm muzzle in world space. */
	FQuat GetMuzzleRotation() const;

	int32 GetRemainingAmmo() const;

	int32 GetRemainingClip() const;

	void ConsumeAmmo();

	const FFirearmStats& GetFirearmStats() const;

	void FireShot();

	virtual void StopFiringEffects();

	UEquippableState* GetFiringState() const;
	UEquippableState* GetChargingState() const;
	UEquippableState* GetReloadingState() const;

	/** AHeroEquippable Interface Begin */
	virtual void PostInitializeComponents() override;
	/** AHeroEquippable Interface End */

protected:
	virtual void PlayFiringEffects();

private:
	UFUNCTION(Server, WithValidation, Reliable)
	void ServerFireShot(FShotData ShotData);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Firearm)
	FFirearmStats Stats;

	int32 RemainingAmmo;

	int32 RemainingClip;

	UPROPERTY(Instanced, EditDefaultsOnly, BlueprintReadOnly, Category = Firearm)
	class UShotType* ShotType;

	UPROPERTY(Instanced, EditDefaultsOnly, BlueprintReadOnly, Category = States)
	UEquippableState* FiringState;

	UPROPERTY(Instanced, EditDefaultsOnly, BlueprintReadOnly, Category = States)
	UEquippableState* ChargingState;

	UPROPERTY(Instanced, EditDefaultsOnly, BlueprintReadOnly, Category = States)
	UEquippableState* ReloadingState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Firearm)
	FName MuzzleSocket;

	// Spawned particle system component for muzzle FX
	UPROPERTY(Transient)
	UParticleSystemComponent* MuzzleFXComponent = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Sound)
	UParticleSystem* MuzzleFX = nullptr;

	// Spawned component used to play FireSound and manipulate looping sounds.
	UPROPERTY(Transient)
	UAudioComponent* FireSoundComponent = nullptr;

	// Sound effect when the weapon is fire. This audio will loop while in the
	// firing weapon state if it is a looping sound. Otherwise, it will be played
	// at the time of each shot. For automatic weapons, it is recommended to use
	// looping firing sounds.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Sound)
	USoundBase* FireSound = nullptr;

	// Sound effect on weapon end fire
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Sound)
	USoundBase* EndFireSound = nullptr;

	// Sound effect on weapon fire with empty clip
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Sound)
	USoundBase* EmptyClipSound = nullptr;
};

FORCEINLINE UEquippableState* AHeroFirearm::GetFiringState() const { return FiringState; }
FORCEINLINE UEquippableState* AHeroFirearm::GetChargingState() const { return ChargingState; }
FORCEINLINE UEquippableState* AHeroFirearm::GetReloadingState() const { return ReloadingState; }
FORCEINLINE int32 AHeroFirearm::GetRemainingAmmo() const { return RemainingAmmo; }
FORCEINLINE int32 AHeroFirearm::GetRemainingClip() const { return RemainingClip; }
FORCEINLINE const FFirearmStats& AHeroFirearm::GetFirearmStats() const { return Stats; }