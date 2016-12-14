// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/GameState.h"
#include "AtomGameState.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API AAtomGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	AAtomGameState();
	
	/** AGameStateBase Interface Begin */
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;
	/** AGameStateBase Interface End */	

public:
	TArray<class AAtomPlayerState*> Teams[2];
};
