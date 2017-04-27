// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "AtomFloatingUI.generated.h"

class UUserWidget;
class SWidget;

UCLASS()
class PROJECTATOMVR_API AAtomFloatingUI : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAtomFloatingUI();

	void SetUMGWidget(UUserWidget* Widget, const FVector2D InResolution, const float InScale);

	void SetSlateWidget(const TSharedRef<SWidget>& Widget, const FVector2D InResolution, const float InScale);

	/** Set visibility of the UI. */
	void ShowUI(const bool bShow);

	/** Get visibility of the UI. */
	bool GetShowUI() const;

	/** AActor Interface Begin */
	virtual void Destroyed() override;
	/** AActor Interface End */

protected:
	void UpdateWidgetComponent();

protected:
	TSharedPtr<SWidget> SlateWidget;

	UPROPERTY()
	UUserWidget* UMGWidget;

	FVector2D Resolution;

	float Scale;

private:
	UPROPERTY()
	class UWidgetComponent* WidgetComponent;
};
