// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "AtomBaseGameMode.h"
#include "AtomLobbyGameMode.generated.h"

/**
 * Game mode that handles the online lobby for the pregame portion of the match. The
 * game mode enters the WaitingPostMatch state once the minimum players for the next match
 * has entered. Then the time stamp is set in the AtomLobbyGameState and once PreGameTimer
 * time has passed, the next match is loaded.
 */
UCLASS(Config=Game)
class PROJECTATOMVR_API AAtomLobbyGameMode : public AAtomBaseGameMode
{
	GENERATED_BODY()
	
public:
	AAtomLobbyGameMode();

	/** AGameModeBase Interface Begin */
	virtual void InitGameState() override;
	virtual bool ReadyToEndMatch_Implementation() override;
	virtual void Tick(float DeltaSeconds) override;
	/** AGameModeBase Interface End */

protected:
	void TravelToNextMatch();

protected:
	/** Delay for match start once player count is meet for next map. */
	UPROPERTY(Config, BlueprintReadOnly)
	int32 PreGameTimer = 30;
};
