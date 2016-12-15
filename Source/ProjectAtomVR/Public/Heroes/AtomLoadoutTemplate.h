// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Object.h"
#include "AtomLoadoutTemplate.generated.h"

USTRUCT(Blueprintable)
struct FAtomLoadoutTemplateSlot
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, Category = Loadout)
	TSubclassOf<class AAtomEquippable> ItemClass;

	/** How many items will be placed in the loadout. */
	UPROPERTY(EditDefaultsOnly, Category = Loadout)
	int32 Count;

	/** Storage location on Hero mesh. */
	UPROPERTY(EditDefaultsOnly, Category = Loadout)
	FName StorageSocket;

	UPROPERTY(EditDefaultsOnly, Category = Loadout)
	FName UISocket;

	/** Radius of overlapping trigger used to equip this item. */
	UPROPERTY(EditDefaultsOnly, Category = Loadout)
	float StorageTriggerRadius;
};

/**
 * 
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class PROJECTATOMVR_API UAtomLoadoutTemplate : public UObject
{
	GENERATED_BODY()
	
public:
	UAtomLoadoutTemplate(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = HeroLoadoutTemplate)
	const TArray<FAtomLoadoutTemplateSlot>& GetLoadoutSlots() const;

	/** UObject Interface Begin */
	virtual void PostLoad() override;
	/** UObject Interface End */

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = HeroLoadoutTemplate, meta = (AllowPrivateAccess = "true"))
	TArray<FAtomLoadoutTemplateSlot> LoadoutSlots;
};
