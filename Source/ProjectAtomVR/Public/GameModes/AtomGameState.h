// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/GameState.h"
#include "AtomGameState.generated.h"

class AAtomPlayerState;

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API AAtomGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	AAtomGameState();

	void SetGameWinner(AAtomPlayerState* Winner);
	AAtomPlayerState* GetGameWinner() const;

	/** AGameState Interface Begin */
	virtual void DefaultTimer() override;
	/** AGameState Interface End */

	/** AGameStateBase Interface Begin */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;	
	/** AGameStateBase Interface End */

public:
	UPROPERTY(Replicated, Transient, BlueprintReadOnly, Category = AtomGameState)
	uint32 bIsTeamGame : 1;

	UPROPERTY(Replicated, Transient, BlueprintReadOnly, Category = AtomGameMode)
	TArray<class AAtomTeamInfo*> Teams; // Only valid if bIsTeamGame

	UPROPERTY(Replicated, Transient, BlueprintReadOnly, Category = AtomGameMode)
	AAtomPlayerState* GameWinner; // Winner of the game.

	UPROPERTY(Replicated, Transient, BlueprintReadOnly, Category = AtomGameMode)
	int32 ScoreLimit; // Score limit for the match

	UPROPERTY(Replicated, Transient, BlueprintReadOnly, Category = AtomGameMode)
	int32 TimeLimit; // Time limit for each round in the match

	UPROPERTY(Replicated, Transient, BlueprintReadOnly, Category = AtomGameMode)
	int32 Rounds = 0;

	UPROPERTY(Replicated, Transient, BlueprintReadOnly, Category = AtomGameMode)
	int32 CurrentRound = 0;

	UPROPERTY(Replicated, Transient, BlueprintReadOnly, Category = AtomGameMode)
	int32 RemainingTime; // Timer for the current MatchState (game timer, intermission timer, countdown timer, etc.)
};
