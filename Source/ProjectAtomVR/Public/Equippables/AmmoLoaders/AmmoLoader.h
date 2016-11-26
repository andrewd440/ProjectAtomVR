// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Object.h"
#include "AmmoLoader.generated.h"


/**
 * 
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class PROJECTATOMVR_API UAmmoLoader : public UObject
{
	GENERATED_BODY()

public:
	UAmmoLoader(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	* Initializes the ammo loader. Called by owning HeroFirearm PostInitializeComponents.
	*/
	virtual void InitializeLoader();

	virtual void SetupInputComponent(class UInputComponent* InputComponent);

	/**
	* Called when the owning HeroFirearm is equipped.
	*/
	virtual void OnEquipped();

	/**
	* Called when the owning HeroFirearm is unequipped.
	*/
	virtual void OnUnequipped();

	UFUNCTION(BlueprintCallable, Category = AmmoLoader)
	int32 GetAmmoCount() const;

	virtual void ConsumeAmmo();

	/**
	* Loads ammo for this loader with an optional load object. To replicate properly,
	* this should only be called externally by HeroOwner::LoadAmmo().
	*/
	virtual void LoadAmmo(UObject* LoadObject);

	/**
	* Attempts to discard ammo for this loader. To replicate properly,
	* this should only be called externally by HeroOwner::DiscardAmmo().
	*/
	virtual bool DiscardAmmo();

	/** UObject Interface Begin */
	virtual bool IsSupportedForNetworking() const override;
	virtual class UWorld* GetWorld() const override final;
	/** UObject Interface End */

protected:
	class AHeroFirearm* GetFirearm() const;

protected:
	int32 AmmoCount = 0;

private:
	class AHeroFirearm* Firearm;
};

FORCEINLINE int32 UAmmoLoader::GetAmmoCount() const { return AmmoCount; }
FORCEINLINE class AHeroFirearm* UAmmoLoader::GetFirearm() const { return Firearm; }