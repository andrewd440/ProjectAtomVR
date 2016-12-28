// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "AtomUIActor.generated.h"

class UUserWidget;
class AAtomUISystem;

UCLASS()
class PROJECTATOMVR_API AAtomUIActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAtomUIActor();

	/** Gets the UI system. May be null in editor. */
	AAtomUISystem* GetUISystem() const;

	/** Sets the UI system. */
	void SetUISystem(AAtomUISystem* UISystem);

	const TArray<UUserWidget*> GetWidgets() const;

	/** AActor Interface Begin */
	virtual void PostInitializeComponents() override;
	virtual void Destroyed() override;
	/** AActor Interface End */

private:
	AAtomUISystem* UISystem = nullptr;

	/** All widgets contained within any widget components. */
	TArray<UUserWidget*> Widgets;
};
