// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "EquippableWidget.generated.h"

class AEquippableUIActor;

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UEquippableWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetOwner(AEquippableUIActor* Owner);

	AEquippableUIActor* GetOwner() { return Owner.Get(); }
	const AEquippableUIActor* GetOwner() const { return Owner.Get(); }

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
	void OnLoadoutChanged(ELoadoutSlotChangeType Change);

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = EquippableWidget)
	TWeakObjectPtr<AEquippableUIActor> Owner;
};
