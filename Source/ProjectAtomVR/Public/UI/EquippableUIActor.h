// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "UI/AtomUIActor.h"
#include "EquippableUIActor.generated.h"

class AHeroEquippable;
class UEquippableWidget;
enum class ELoadoutSlotChangeType : uint8;


/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API AEquippableUIActor : public AAtomUIActor
{
	GENERATED_BODY()
	
public:
	AEquippableUIActor();

	/**
	* Called when the loadout slot for the owning equippable has changed. Type 
	* will never be Item as that is handled externally.
	*/
	void OnLoadoutChanged(ELoadoutSlotChangeType Type);

	/** AActor Interface Begin */
	virtual void PostInitializeComponents() override;
	virtual void SetOwner(AActor* NewOwner) override;
	/** AActor Interface End */

protected:
	const TArray<UEquippableWidget*>& GetWidgets() const;

	/**
	* Called when the equip status of the owning equippable has changed.
	* @returns
	*/
	void OnEquippedStatusChanged();

private:
	TArray<UEquippableWidget*> EquippableWidgets;
};

FORCEINLINE const TArray<UEquippableWidget*>& AEquippableUIActor::GetWidgets() const { return EquippableWidgets; }