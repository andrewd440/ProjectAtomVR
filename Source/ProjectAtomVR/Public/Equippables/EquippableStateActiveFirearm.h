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
	virtual void OnMagazineAttachmentChanged();

	/** UEquippable Interface Begin */
public:
	virtual void OnEnteredState() override;
	virtual void OnStatePushed() override;
	virtual void OnStatePopped() override;	
protected:
	virtual void BindStateInputs(UInputComponent* InputComponent) override;
	/** UEquippable Interface End */

private:
	FDelegateHandle OnClipMagazineHandle;
};