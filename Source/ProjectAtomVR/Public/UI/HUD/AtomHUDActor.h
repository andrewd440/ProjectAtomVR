// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "AtomHUDActor.generated.h"

class UUserWidget;
class AVRHUD;

UCLASS()
class PROJECTATOMVR_API AAtomHUDActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAtomHUDActor();

	/** Gets the UI system. May be null in editor. */
	AVRHUD* GetHUD() const;

	const TArray<UUserWidget*> GetWidgets() const;

	/** AActor Interface Begin */
	virtual void PostInitializeComponents() override;
	virtual void Destroyed() override;
	/** AActor Interface End */

private:
	/** All widgets contained within any widget components. */
	TArray<UUserWidget*> Widgets;
};
