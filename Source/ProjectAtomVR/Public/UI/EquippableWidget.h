// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "EquippableWidget.generated.h"

class AEquippableUIActor;

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UEquippableWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetOwner(AEquippableUIActor* Owner);

	AEquippableUIActor* GetOwner() { return Owner.Get(); }
	const AEquippableUIActor* GetOwner() const { return Owner.Get(); }

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = EquippableWidget)
	TWeakObjectPtr<AEquippableUIActor> Owner;
};
