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
	/** EquippableState Interface Begin */
	virtual void OnEnteredState() override;
	virtual void OnExitedState() override;
	/** EquippableState Interface End */

protected:
	UFUNCTION()
	virtual void OnClipEnteredReloadTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnClipExitedReloadTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
