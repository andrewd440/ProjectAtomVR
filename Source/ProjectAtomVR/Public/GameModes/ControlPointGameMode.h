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

	/** AtomTeamGameMode Interface Begin */
public:
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	/** AtomTeamGameMode Interface End */

	/** AtomGameMode Interface Begin */
protected:
	virtual void OnObjectiveInitialized(class AAtomGameObjective* Objective) override;
	virtual void InitGameStateForRound(AAtomGameState* GameState) override;
	/** AtomGameMode Interface End */

protected:
	UPROPERTY(EditDefaultsOnly, Category = ControlPoint)
	int32 ObjectiveSpawnDelay = 30;

	UPROPERTY(BlueprintReadOnly, Category = ControlPoint)
	TArray<class AAtomControlPoint*> GameControlPoints; // Ordered control points for each round	
};
