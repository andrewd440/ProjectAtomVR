// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/PlayerController.h"
#include "AtomPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API AAtomPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AAtomPlayerController();

	AHeroBase* GetHero() const;

	/** APlayerController Interface Begin */
	virtual void PostInitializeComponents() override;
	virtual void SetPawn(APawn* aPawn) override;
	/** APlayerController Interface End */
	
protected:
	void CreateUISystem();

private:
	UPROPERTY()
	class UAtomUISystem* UISystem = nullptr;
	class AHeroBase* Hero = nullptr;
};
