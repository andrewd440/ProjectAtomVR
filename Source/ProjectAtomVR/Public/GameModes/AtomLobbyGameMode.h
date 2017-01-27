// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "AtomTeamGameMode.h"
#include "AtomLobbyGameMode.generated.h"

/**
 * Game mode that handles the online lobby for the pregame portion of the match. The
 * game mode enters the WaitingPostMatch state once the minimum players for the next match
 * has entered. Then the time stamp is set in the AtomLobbyGameState and once PreGameTimer
 * time has passed, the next match is loaded.
 */
UCLASS(Config=Game)
class PROJECTATOMVR_API AAtomLobbyGameMode : public AAtomTeamGameMode
{
	GENERATED_BODY()
	
public:
	AAtomLobbyGameMode();

	/** AAtomGameMode Interface Begin */
public:
	virtual bool CanDamage_Implementation(AController* Inflictor, AController* Reciever) const override;
protected:
	virtual void TravelToNextMatch() override;
	virtual bool IsCharacterChangeAllowed_Implementation(class AAtomPlayerController* Controller) const override;
	/** AAtomGameMode Interface End */

	/** AGameModeBase Interface Begin */
public:
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void InitGameState() override;
	virtual void Tick(float DeltaSeconds) override;
	/** AGameModeBase Interface End */

protected:
	/** Delay for match start once conditions as been met for the next match to start. */
	UPROPERTY(Config, BlueprintReadOnly)
	int32 PreGameTimer = 30;
};
