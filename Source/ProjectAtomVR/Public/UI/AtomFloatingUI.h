// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "UI/AtomUIActor.h"
#include "AtomFloatingUI.generated.h"

class UUserWidget;
class UAtomWidgetComponent;
class AAtomUISystem;

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API AAtomFloatingUI : public AAtomUIActor
{
	GENERATED_BODY()
	
public:
	AAtomFloatingUI();

	void SetWidget(AAtomUISystem* UISystem, TSubclassOf<UUserWidget> InWidget);
	
	/** AAtomFloatingUI Interface Begin */
	virtual void Tick(float DeltaSeconds) override;
	/** AAtomFloatingUI Interface End */

protected:
	void SetupWidgetComponent();

protected:
	/** The widget being drawn. */
	UPROPERTY()
	UUserWidget* Widget;

	UPROPERTY()
	TSubclassOf<UUserWidget> WidgetClass;

	/** Component used to display Widget. */
	UPROPERTY(BlueprintReadOnly, Category = AtomFloatingUI)
	UAtomWidgetComponent* WidgetComponent;

	AAtomUISystem* Owner;
};
