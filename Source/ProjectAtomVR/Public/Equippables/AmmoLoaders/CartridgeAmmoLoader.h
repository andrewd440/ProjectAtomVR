// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Equippables/AmmoLoaders/AmmoLoader.h"
#include "CartridgeAmmoLoader.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UCartridgeAmmoLoader : public UAmmoLoader
{
	GENERATED_BODY()
	
public:
	UCartridgeAmmoLoader(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** UAmmoLoader Interface Begin */
	virtual void LoadAmmo(UObject* LoadObject) override;
	virtual void OnEquipped() override;
	virtual void OnUnequipped() override;
	virtual void ConsumeAmmo() override;
	virtual void InitializeLoader() override;
	/** UAmmoLoader Interface End */

	/** UObject Interface Begin */
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> & OutLifetimeProps) const override;
	/** UObject Interface End */

protected:
	UFUNCTION()
	void OnHandEnteredReloadTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	/** The type of cartridge this firearm uses.*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = CartridgeAmmoLoader)
	TSubclassOf<class AHeroEquippable> CartridgeType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = CartridgeAmmoLoader)
	int32 Capacity = 10;

private:
	/** Trigger used to determine valid overlap for a cartridge to be loaded.*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = CartridgeAmmoLoader, meta = (AllowPrivateAccess = "true"))
	USphereComponent* LoadTrigger;
};
