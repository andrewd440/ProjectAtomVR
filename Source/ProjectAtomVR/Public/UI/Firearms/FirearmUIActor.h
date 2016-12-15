// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "EquippableUIActor.h"
#include "FirearmUIActor.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API AFirearmUIActor : public AEquippableUIActor
{
	GENERATED_BODY()
	
public:
	AFirearmUIActor();

	UFUNCTION(BlueprintCallable, Category = FirearmUIActor)
	class AAtomFirearm* GetFirearm() const;

	/** AEquippableUIActor Interface Begin */
	virtual void PostInitializeComponents() override;	
	/** AEquippableUIActor Interface End */

	/** AActor Interface Begin */
	virtual void BeginPlay() override;
	/** AActor Interface End */

protected:
	void OnAmmoCountChanged();

private:
	TArray<class UFirearmWidget*> FirearmWidgets;
};
