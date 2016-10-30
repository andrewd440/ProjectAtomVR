// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Equippables/EquippableState.h"
#include "EquippableState_ClipReload.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UEquippableState_ClipReload : public UEquippableState
{
	GENERATED_BODY()
	
public:
	UEquippableState_ClipReload(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** EquippableState Interface Begin */
	virtual void OnEnteredState() override;
	virtual void OnExitedState() override;
	/** EquippableState Interface End */

protected:
	UFUNCTION()
	virtual void OnClipEnteredReloadTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	/** Does the clip need to be in hand to load into the firearm. */
	UPROPERTY(EditDefaultsOnly, Category = ReloadState)
	uint32 bRequiresEquippedClip : 1;
};
