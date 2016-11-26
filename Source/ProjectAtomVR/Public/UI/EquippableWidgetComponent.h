// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Components/WidgetComponent.h"
#include "EquippableWidgetComponent.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, ClassGroup = "UserInterface", hidecategories = (Object, Activation, "Components|Activation", Base, Lighting, LOD, Mesh), editinlinenew, meta = (BlueprintSpawnableComponent))
class PROJECTATOMVR_API UEquippableWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	FName GetEquippableAttachSocket() const { return EquippableAttachSocket; }

protected:
	UPROPERTY(EditDefaultsOnly, Category = EquippableWidget)
	FName EquippableAttachSocket = NAME_None;
};
