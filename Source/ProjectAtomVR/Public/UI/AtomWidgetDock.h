// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "UI/AtomFloatingDock.h"
#include "AtomLocalMessageInterface.h"
#include "AtomWidgetDock.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API AAtomWidgetDock : public AAtomFloatingDock
{
	GENERATED_BODY()
	
public:

	AAtomWidgetDock();
	
	void SetUMGWidget(class UUserWidget* Widget, const FVector2D InResolution, const float InScale);

	void RecieveLocalMessage(TSubclassOf<class UAtomLocalMessage> MessageClass, const int32 MessageIndex, const FText& MessageText,
		class AAtomPlayerState* RelatedPlayerState_1, class AAtomPlayerState* RelatedPlayerState_2, UObject* OptionalObject);

	/** AAtomFloatingDock Interface Begin */
public:
	virtual void Deactivate(const float InDelay = 0.f) override;
protected:
	virtual void PostExtended() override;
	virtual void UpdateInternal(const FVector& OrientateToward, const FQuat& TowardRotation) override;
	/** AAtomFloatingDock Interface End */

private:
	UPROPERTY()
	class UWidgetComponent* WidgetComponent;
};
