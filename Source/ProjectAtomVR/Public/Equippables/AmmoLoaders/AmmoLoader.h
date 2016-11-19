// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "UObject/NoExportTypes.h"
#include "AmmoLoader.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class PROJECTATOMVR_API UAmmoLoader : public UObject, public FTickableGameObject
{
	GENERATED_BODY()
	
public:
	UAmmoLoader(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void InitializeLoader();

	virtual void SetupInputComponent(class UInputComponent* InputComponent);

	virtual void OnEquipped();

	virtual void OnUnequipped();

	int32 GetAmmoCount() const;

	virtual void ConsumeAmmo();

	virtual void LoadAmmo(UObject* LoadObject);

	virtual bool DiscardAmmo();

	/** UObject Interface Begin */
	virtual bool IsSupportedForNetworking() const override;
	virtual class UWorld* GetWorld() const override final;
	/** UObject Interface End */

	/** FTickableGameObject Interface Begin */
	virtual UWorld* GetTickableGameObjectWorld() const override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;
	/** FTickableGameObject Interface End */

protected:
	class AHeroFirearm* GetFirearm() const;

protected:
	int32 AmmoCount = 0;

private:
	class AHeroFirearm* Firearm;
};

FORCEINLINE int32 UAmmoLoader::GetAmmoCount() const { return AmmoCount; }
FORCEINLINE class AHeroFirearm* UAmmoLoader::GetFirearm() const { return Firearm; }