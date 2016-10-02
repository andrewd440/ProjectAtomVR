// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Components/CapsuleComponent.h"
#include "HMDCapsuleComponent.generated.h"

/**
 * Supported Attachment:
 *	VRRoot
 *		|- UCameraComponent
 *			|- UHMDCapsuleComponent
 */
UCLASS()
class PROJECTATOMVR_API UHMDCapsuleComponent : public UCapsuleComponent
{
	GENERATED_BODY()
	
public:
	UHMDCapsuleComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** UCapsuleComponent Interface Begin */
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	/** UCapsuleComponent Interface End */	

	/** UActorComponent Interface Begin */
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	/** UActorComponent Interface End */

	/** UPrimitiveComponent Interface Begin */
protected:
	virtual void OnAttachmentChanged() override;
	/** UPrimitiveComponent Interface End */
};
