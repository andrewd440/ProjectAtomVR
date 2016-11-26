// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "UI/AtomUIActor.h"
#include "EquippableUIActor.generated.h"

class AHeroEquippable;
class UEquippableWidget;

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API AEquippableUIActor : public AAtomUIActor
{
	GENERATED_BODY()
	
public:

	/** AActor Interface Begin */
	virtual void PostInitializeComponents() override;
	/** AActor Interface End */

protected:
	const TArray<UEquippableWidget*>& GetWidgets() const;

private:
	TArray<UEquippableWidget*> EquippableWidgets;
};

FORCEINLINE const TArray<UEquippableWidget*>& AEquippableUIActor::GetWidgets() const { return EquippableWidgets; }