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
		if (GameControlPoints.Num() > 0)
		{
			ControlPointGameState->SetActiveControlPoint(GameControlPoints[0]);
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
