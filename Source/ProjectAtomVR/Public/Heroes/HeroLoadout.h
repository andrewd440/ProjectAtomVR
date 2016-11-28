// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Object.h"
#include "HeroLoadout.generated.h"

UENUM(BlueprintType, meta = (Bitflags))
enum class ELoadoutSlotChangeType : uint8
{
	None =	0x00,
	Item =	0x01,
	Count = 0x02
};

ENUM_CLASS_FLAGS(ELoadoutSlotChangeType);

USTRUCT()
struct FHeroLoadoutSlot
{
	GENERATED_USTRUCT_BODY()

	/** Called when the loadout slot item or count is changed. Should only be bound by
	 ** the slot UI actor. */
	DECLARE_DELEGATE_OneParam(FItemChanged, ELoadoutSlotChangeType)
	FItemChanged OnSlotChanged;

	UPROPERTY(BlueprintReadOnly)
	class AHeroEquippable* Item = nullptr;

	// Remaining items for this slot
	UPROPERTY(BlueprintReadOnly)
	int32 Count = 0;

	// Socket the item is attached to when in storage
	FName StorageSocket;

	// Socket the item UI is attached to
	FName UISocket;

	// The trigger volume for the item's storage location
	class USphereComponent* StorageTrigger = nullptr;
};

/**
 * 
 */
UCLASS(DefaultToInstanced, EditInlineNew)
class PROJECTATOMVR_API UHeroLoadout : public UObject
{
	GENERATED_BODY()
	
public:
	static const FHeroLoadoutSlot NullLoadoutSlot;

public:
	/**
	 * Initializes loadout slots. Does not spawn any loadout items.
	 * Should be called by the owning hero on PostInitializeComponents.
	 */
	void InitializeLoadout(class AHeroBase* Owner);	

	/** 
	 * Spawns all loadout items. Should be called after the owning hero as been possessed by
	 * a controller (i.e. BeginPlay). 
	 */
	void SpawnLoadout();

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

	/** Gets the loadout slots. */
	const TArray<FHeroLoadoutSlot>& GetLoadoutSlots() const;

	/** Gets the loadout slots. */
	TArray<FHeroLoadoutSlot>& GetLoadoutSlots();

	/** Gets the loadout template. Template items and loadout slots map one-to-one. */
	const TSubclassOf<class UHeroLoadoutTemplate> GetLoadoutTemplate() const;

	UFUNCTION(BlueprintCallable, Category = Loadout)
	const FHeroLoadoutSlot& GetItemSlot(const class AHeroEquippable* Item) const;

	USceneComponent* GetAttachParent() const;

	/** UObject Interface Begin */
	virtual void PreNetReceive() override;
	/** UObject Interface End */	

protected:

	/**
	* Only bound on the server. Called when a loadout item changes it's return to loadout
	* property.
	*/
	void OnReturnToLoadoutChanged(class AHeroEquippable* Item, int32 LoadoutIndex);

	UFUNCTION()
	void OnRep_Loadout();

private:
	/** Creates all loadout weapons. Should only be called on server. */
	void CreateLoadoutEquippables(const TArray<struct FHeroLoadoutTemplateSlot>& LoadoutTemplateSlots);

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
	UPROPERTY(ReplicatedUsing=OnRep_Loadout, VisibleAnywhere, Category = HeroLoadout)
	TArray<FHeroLoadoutSlot> Loadout;

	class AHeroBase* HeroOwner = nullptr;
};
