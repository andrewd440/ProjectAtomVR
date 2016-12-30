// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/GameState.h"
#include "AtomGameState.generated.h"

class AAtomPlayerState;

USTRUCT()
struct FAtomTeam
{
	GENERATED_USTRUCT_BODY()

	TArray<AAtomPlayerState*> Players;

	UPROPERTY(BlueprintReadOnly)
	int32 Score;
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
	
	void ScoreKill(AAtomPlayerState* Player, int32 Score);

	void ScoreDeath(AAtomPlayerState* Player, int32 Score);

	/** AGameStateBase Interface Begin */
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	/** AGameStateBase Interface End */

protected:
	UPROPERTY(Replicated)
	FAtomTeam Teams[2];
};
