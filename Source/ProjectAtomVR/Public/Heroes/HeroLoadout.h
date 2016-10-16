// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Object.h"
#include "HeroLoadout.generated.h"

USTRUCT()
struct FHeroLoadoutSlot
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	class AHeroEquippable* Item;

	// Socket the item is attached to when in storage
	FName StorageSocket;

	// The trigger volume for the item's storage location
	class USphereComponent* StorageTrigger;
};

/**
 * 
 */
UCLASS(DefaultToInstanced, EditInlineNew)
class PROJECTATOMVR_API UHeroLoadout : public UObject
{
	GENERATED_BODY()
	
public:
	/**
	 * Initializes the loadout. Should be called by the owning hero on BeginPlay.
	 */
	void InitializeLoadout(class AHeroBase* Owner);	

	/**
	* Requests an equip from the loadout. OverlapComponent will be used to check for
	* overlaps with loadout slots to see if the component is within the bounds of a loadout
	* item. If successful, the AHeroBase::Equip will be called on the owning hero with the 
	* corresponding loadout item.
	**/
	bool RequestEquip(UPrimitiveComponent* OverlapComponent, const EHand Hand);

	/**
	 * Requests an unequip from the loadout. OverlapComponent will be used to check for
	 * overlaps with loadout slots to see if the component is in the correct location to
	 * unequip the specified item. If successful, the AHeroBase::Unequip will be called on
	 * the owning hero.
	 **/
	bool RequestUnequip(UPrimitiveComponent* OverlapComponent, AHeroEquippable* Item);

private:
	/** Creates all loadout weapons. Should only be called on server. */
	void CreateLoadoutWeapons(const TArray<struct FHeroLoadoutTemplateSlot>& LoadoutTemplateSlots);

	/** Creates all loadout item triggers. Should only be called on controlling players. */
	void CreateLoadoutTriggers(const TArray<struct FHeroLoadoutTemplateSlot>& LoadoutTemplateSlots);
	
	/** UObject Interface Begin */
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> & OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override;
	virtual class UWorld* GetWorld() const override;
	/** UObject Interface End */

private:
	/** 
	 * Called when the player's hand overlaps one of the loadout triggers. 
	 */
	UFUNCTION()
	void OnLoadoutTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	UPROPERTY(EditDefaultsOnly, Category = HeroLoadout)
	TSubclassOf<class UHeroLoadoutTemplate> LoadoutTemplate;

	/** Haptic played when the players' hand overlaps an equippable item. */
	UPROPERTY(EditDefaultsOnly, Category = HeroLoadout)
	class UHapticFeedbackEffect_Base* TriggerFeedback;

private:
	UPROPERTY(Replicated, VisibleAnywhere, Category = HeroLoadout)
	TArray<FHeroLoadoutSlot> Loadout;

	class AHeroBase* HeroOwner = nullptr;
};
