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

	/** AGameStateBase Interface Begin */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	/** AGameStateBase Interface End */

public:
	UPROPERTY(Replicated, BlueprintReadOnly, Category = AtomGameState)
	uint32 bIsTeamGame : 1;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = AtomGameMode)
	TArray<class AAtomTeamInfo*> Teams; // Only valid if bIsTeamGame

	UPROPERTY(Replicated, BlueprintReadOnly, Category = AtomGameMode)
	AAtomPlayerState* GameWinner; // Winner of the game.

	UPROPERTY(Replicated, BlueprintReadOnly, Category = AtomGameMode)
	int32 ScoreLimit;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = AtomGameMode)
	int32 TimeLimit;
};
