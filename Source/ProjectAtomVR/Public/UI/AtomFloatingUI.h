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
	
	enum class EDockMode : uint8
	{
		Player, // Docked on the player
		World // Docked in the world
	};

public:	
	// Sets default values for this actor's properties
	AAtomFloatingUI();

	void SetUMGWidget(TSubclassOf<UUserWidget> Widget);

	void SetSlateWidget(const TSharedRef<SWidget>& Widget);

	/** AActor Interface Begin */
	virtual void Destroyed() override;
	/** AActor Interface End */

protected:
	void UpdateWidgetComponent();

protected:
	TSharedPtr<SWidget> SlateWidget;

	TSubclassOf<UUserWidget> UMGWidgetClass;

	UPROPERTY()
	UUserWidget* UMGWidget;

private:
	UPROPERTY()
	class UWidgetComponent* WidgetComponent;
};
