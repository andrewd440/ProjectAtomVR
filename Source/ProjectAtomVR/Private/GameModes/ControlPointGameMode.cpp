// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "ControlPointGameMode.h"
#include "AtomGameObjective.h"
#include "AtomControlPoint.h"
#include "ControlPointGameState.h"

AControlPointGameMode::AControlPointGameMode()
{
	GameStateClass = AControlPointGameState::StaticClass();
}

void AControlPointGameMode::InitGameStateForRound(AAtomGameState* InGameState)
{
	Super::InitGameStateForRound(InGameState);

	if (auto ControlPointGameState = Cast<AControlPointGameState>(InGameState))
	{
		const int32 CurrentRound = ControlPointGameState->CurrentRound;
		const int32 NumControlPoints = GameControlPoints.Num();
		if (NumControlPoints > 0)
		{
			const int32 ObjectiveIndex = (CurrentRound - 1) % NumControlPoints;
			ControlPointGameState->SetActiveControlPoint(GameControlPoints[ObjectiveIndex]);
		}
	}
}

void AControlPointGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	if (auto ControlPointGameState = Cast<AControlPointGameState>(GameState))
	{
		AAtomControlPoint* ControlPoint = ControlPointGameState->GetActiveControlPoint();

		if (ControlPoint)
		{
			ControlPoint->Activate(ObjectiveSpawnDelay);
		}
	}
}

void AControlPointGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// Randomize order of control points
	const int32 LastIndex = GameControlPoints.Num() - 1;
	for (int32 i = 0; i <= LastIndex; ++i)
	{
		int32 SwapIndex = FMath::RandRange(0, LastIndex);
		if (SwapIndex != i)
		{
			GameControlPoints.SwapMemory(i, SwapIndex);
		}
	}
}

void AControlPointGameMode::OnObjectiveInitialized(class AAtomGameObjective* Objective)
{
	Super::OnObjectiveInitialized(Objective);

	if (auto ControlPoint = Cast<AAtomControlPoint>(Objective))
	{
		GameControlPoints.Add(ControlPoint);
	}	
}
