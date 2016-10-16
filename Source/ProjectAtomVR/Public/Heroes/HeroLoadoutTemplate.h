// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Object.h"
#include "HeroLoadoutTemplate.generated.h"

USTRUCT(Blueprintable)
struct FHeroLoadoutTemplateSlot
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, Category = Loadout)
	TSubclassOf<class AHeroEquippable> Item;

	UPROPERTY(EditDefaultsOnly, Category = Loadout)
	FName StorageSocket;

	UPROPERTY(EditDefaultsOnly, Category = Loadout)
	float StorageTriggerRadius;
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
