// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "AtomHUDActor.h"
#include "EquippableHUDActor.generated.h"

class AAtomEquippable;
class UEquippableWidget;
enum class ELoadoutSlotChangeType : uint8;

class AAtomEquippable;

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API AEquippableHUDActor : public AAtomHUDActor
{
	GENERATED_BODY()
	
public:
	AEquippableHUDActor();

	/**
	* Called when the loadout slot for the owning equippable has changed. Type 
	* will never be Item as that is handled externally.
	*/
	void OnLoadoutChanged(ELoadoutSlotChangeType Type, const struct FAtomLoadoutSlot& LoadoutSlot);

	UFUNCTION(BlueprintCallable, Category = EquippableUI)
	AAtomEquippable* GetEquippable() const;

	void SetEquippable(AAtomEquippable* NewEquippable);

	/** AActor Interface Begin */
	virtual void PostInitializeComponents() override;
	virtual void SetOwner(AActor* NewOwner) override;
	virtual void Destroyed() override;
	/** AActor Interface End */

protected:
	const TArray<UEquippableWidget*>& GetEquippableWidgets() const;

	void UpdateWidgetAttachments();

	/**
	* Called when the equip status of the owning equippable has changed.
	* @returns
	*/
	void OnEquippedStatusChanged();

private:
	TArray<UEquippableWidget*> EquippableWidgets;

	TWeakObjectPtr<AAtomEquippable> Equippable = nullptr;
};

FORCEINLINE const TArray<UEquippableWidget*>& AEquippableHUDActor::GetEquippableWidgets() const { return EquippableWidgets; }