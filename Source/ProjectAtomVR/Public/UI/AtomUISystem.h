// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "UObject/NoExportTypes.h"
#include "AtomUISystem.generated.h"

enum class ELoadoutSlotChangeType : uint8;
class AAtomPlayerController;

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UAtomUISystem : public UObject
{
	GENERATED_BODY()
	
public:
	UAtomUISystem();

	void SetOwner(AAtomPlayerController* Owner);
	AAtomPlayerController* GetOwner() const;
	AAtomCharacter* GetCharacter() const;

	void SpawnCharacterUI();
	void DestroyCharacterUI();
	
	/** UObject Interface Begin */
	virtual class UWorld* GetWorld() const override;
	/** UObject Interface End */	


private:
	void OnLoadoutSlotChanged(ELoadoutSlotChangeType Change, int32 LoadoutIndex);

private:
	AAtomPlayerController* Owner;

	struct FHeroUI
	{
		TArray<AEquippableUIActor*> Equippables;			

		// Hero (UI on the hero only, i.e. health)
	} HeroUI;
};
