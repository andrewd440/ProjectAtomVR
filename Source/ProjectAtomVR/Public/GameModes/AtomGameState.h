// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/GameState.h"
#include "AtomGameState.generated.h"

struct FAtomTeam
{
	TArray<class AAtomPlayerState*> Players;
	int Score;
};

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API AAtomGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	AAtomGameState();
	
	void AddTeamScore(int TeamId, int Score);

	/** AGameStateBase Interface Begin */
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;
	/** AGameStateBase Interface End */	

public:
	FAtomTeam Teams[2];
};
