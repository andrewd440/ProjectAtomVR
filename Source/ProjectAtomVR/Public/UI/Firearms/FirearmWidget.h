// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "FirearmWidget.generated.h"

class AFirearmUIActor;

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
	class AFirearmUIActor* GetFirearmUI();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = FirearmWidget)
	class AHeroFirearm* GetFirearm();
};
