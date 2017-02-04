// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "AtomTeamGameMode.h"
#include "ControlPointGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API AControlPointGameMode : public AAtomTeamGameMode
{
	GENERATED_BODY()
	
public:
	AControlPointGameMode();

protected:
	void OnControlPointCaptured();	

	/** AtomTeamGameMode Interface Begin */
public:
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	/** AtomTeamGameMode Interface End */

	/** AtomGameMode Interface Begin */
protected:	
	virtual void ScoreKill_Implementation(AAtomPlayerState* Killer, AAtomPlayerState* Victim) override;
	virtual void OnObjectiveInitialized(class AAtomGameObjective* Objective) override;
	virtual void InitGameStateForRound(AAtomGameState* GameState) override;
	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchEnteredIntermission() override;
	virtual void HandleMatchHasEnded() override;
	virtual void HandleMatchLeavingIntermission() override;
	/** AtomGameMode Interface End */

	/** AAtomBaseGameMode Interface Begin */
protected:
	virtual void DefaultTimer() override;
	/** AAtomBaseGameMode Interface End */

protected:
	UPROPERTY(EditDefaultsOnly, Category = ControlPoint)
	int32 ObjectiveSpawnDelay = 30;

	UPROPERTY(EditDefaultsOnly, Category = ControlPoint)
	int32 AttackKillScore = 10; // Score awarded to players for kills obtained while capturing the point.

	UPROPERTY(EditDefaultsOnly, Category = ControlPoint)
	int32 DefenseKillScore = 10; // Score awarded to players for kills obtained while defending the point.

	UPROPERTY(EditDefaultsOnly, Category = ControlPoint)
	int32 CaptureScore = 10; // Score awarded to players for capturing a point.

	UPROPERTY(EditDefaultsOnly, Category = ControlPoint)
	int32 ControlScoreRate = 1; // Score rate awarded to the controlling team per second the point is captured.

	UPROPERTY(BlueprintReadOnly, Category = ControlPoint)
	TArray<class AAtomControlPoint*> GameControlPoints; // Ordered control points for each round	

	FDelegateHandle ControlPointCapturedHandle;
};
