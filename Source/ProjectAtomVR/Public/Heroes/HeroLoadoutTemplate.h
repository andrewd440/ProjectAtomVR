// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Object.h"
#include "HeroLoadoutTemplate.generated.h"

USTRUCT(Blueprintable)
struct FHeroLoadoutTemplateSlot
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, Category = Loadout)
	TSubclassOf<class AHeroEquippable> ItemClass;

	/** How many items will be placed in the loadout. */
	UPROPERTY(EditDefaultsOnly, Category = Loadout)
	int32 Count;

	/** Storage location on Hero mesh. */
	UPROPERTY(EditDefaultsOnly, Category = Loadout)
	FName StorageSocket;

	/** Radius of overlapping trigger used to equip this item. */
	UPROPERTY(EditDefaultsOnly, Category = Loadout)
	float StorageTriggerRadius;

	/*UPROPERTY(EditDefaultsOnly, Category = Loadout)
	TSubclassOf<class ALoadoutSlotUI> SlotUI;*/
};

/**
 * 
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class PROJECTATOMVR_API UHeroLoadoutTemplate : public UObject
{
	GENERATED_BODY()
	
public:
	UHeroLoadoutTemplate(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = HeroLoadoutTemplate)
	const TArray<FHeroLoadoutTemplateSlot>& GetLoadoutSlots() const;

	/** UObject Interface Begin */
	virtual void PostLoad() override;
	/** UObject Interface End */

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = HeroLoadoutTemplate, meta = (AllowPrivateAccess = "true"))
	TArray<FHeroLoadoutTemplateSlot> LoadoutSlots;
};
