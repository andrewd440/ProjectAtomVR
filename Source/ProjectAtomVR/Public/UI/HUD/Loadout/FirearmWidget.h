// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "EquippableWidget.h"
#include "FirearmWidget.generated.h"

class AFirearmHUDActor;

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UFirearmWidget : public UEquippableWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, Category = FirearmWidget)
	void OnAmmoCountChanged();	

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = FirearmWidget)
	class AFirearmHUDActor* GetFirearmUI();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = FirearmWidget)
	class AAtomFirearm* GetFirearm();
};
