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
	void InitializeLoadout(class AHeroBase* Owner);	

	bool RequestEquip(UPrimitiveComponent* OverlapComponent, const EHand Hand);

	bool RequestUnequip(UPrimitiveComponent* OverlapComponent, AHeroEquippable* Item);

private:
	void CreateLoadoutWeapons(const TArray<struct FHeroLoadoutTemplateSlot>& LoadoutTemplateSlots);
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
