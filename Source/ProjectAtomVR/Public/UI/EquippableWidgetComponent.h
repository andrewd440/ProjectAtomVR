// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Components/WidgetComponent.h"
#include "EquippableWidgetComponent.generated.h"

UENUM()
enum class EEquippableWidgetType : uint8
{
	Item, // The widget is attached to the equippable item
	Loadout, // The widget is attached to the loadout slot for the item
};

/**
 * 
 */
UCLASS(Blueprintable, ClassGroup = "UserInterface", hidecategories = (Object, Activation, "Components|Activation", Base, Lighting, LOD, Mesh), editinlinenew, meta = (BlueprintSpawnableComponent))
class PROJECTATOMVR_API UEquippableWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	FName GetEquippableAttachSocket() const { return WidgetAttachSocket; }

	EEquippableWidgetType GetWidgetType() const { return Type; }

protected:
	UPROPERTY(EditDefaultsOnly, Category = EquippableWidget)
	EEquippableWidgetType Type = EEquippableWidgetType::Item;

	UPROPERTY(EditDefaultsOnly, Category = EquippableWidget)
	FName WidgetAttachSocket = NAME_None;
};
