// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "EquippableHUDActor.h"
#include "FirearmHUDActor.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API AFirearmHUDActor : public AEquippableHUDActor
{
	GENERATED_BODY()
	
public:
	AFirearmHUDActor();

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
