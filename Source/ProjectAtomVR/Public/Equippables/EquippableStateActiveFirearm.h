// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "EquippableStateActive.h"
#include "EquippableStateActiveFirearm.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UEquippableStateActiveFirearm : public UEquippableStateActive
{
	GENERATED_BODY()

protected:
	virtual void OnTriggerPressed();
	virtual void OnEjectClip();
	void OnClipAttachmentChanged();

private:
	UFUNCTION()
	void OnRep_IsFiring();

	/** UEquippable Interface Begin */
public:
	virtual void OnEnteredState() override;
	virtual void OnStatePopped() override;
	virtual void OnStatePushed() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> & OutLifetimeProps) const override;
	virtual void OnExitedState() override;

protected:
	virtual void BindStateInputs(UInputComponent* InputComponent) override;
	/** UEquippable Interface End */

private:
	FDelegateHandle OnClipChangedHandle;

	// Used on server to notify clients of firing event
	UPROPERTY(ReplicatedUsing = OnRep_IsFiring)
	uint32 bIsFiring : 1;
};