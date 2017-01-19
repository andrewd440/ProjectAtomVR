// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "AtomLoadout.h"

#include "EquippableWidget.generated.h"

class AEquippableHUDActor;

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UEquippableWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetOwner(AEquippableHUDActor* Owner);

	AEquippableHUDActor* GetOwner() { return Owner.Get(); }
	const AEquippableHUDActor* GetOwner() const { return Owner.Get(); }

	/**
	* Called when the equippable has been equipped.
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = EquippableWidget)
	void OnEquipped();

	/**
	* Called when the equippable has been unequipped.
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = EquippableWidget)
	void OnUnequipped();

	/**
	* Called when a new equippable as been assigned to the owning UI actor.
	* @returns
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = EquippableWidget)
	void OnNewEquippable();

	/**
	* Called when the loadout slot for the equippable has changed. Change will never
	* be Item, as that is handle externally.
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = EquippableWidget)
	void OnLoadoutChanged(ELoadoutSlotChangeType Change, const FAtomLoadoutSlot& LoadoutSlot);

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = EquippableWidget)
	TWeakObjectPtr<AEquippableHUDActor> Owner;
};
