// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "ControlPointGameMode.h"
#include "AtomGameObjective.h"
#include "AtomControlPoint.h"
#include "ControlPointGameState.h"
#include "AtomTeamInfo.h"
#include "AtomCharacter.h"
#include "AtomPlayerState.h"
#include "ControlPointPlayerState.h"
#include "AtomObjectiveMessage.h"

AControlPointGameMode::AControlPointGameMode()
{
	GameStateClass = AControlPointGameState::StaticClass();

	KillScore = 0;
	DeathScore = 0;
	ScoreLimit = 100;
}

void AControlPointGameMode::OnControlPointCaptured()
{
	// Get team members and add captured score
	auto CPGameState = CastChecked<AControlPointGameState>(GameState);
	AAtomControlPoint* ControlPoint = CPGameState->GetActiveControlPoint();
	check(ControlPoint);

	TArray<AAtomPlayerState*> CapturePlayers = ControlPoint->GetActiveControllingTeamMembers();
	for (AAtomPlayerState* Player : CapturePlayers)
	{
		if (auto CPPlayerState = Cast<AControlPointPlayerState>(Player))
		{			
			++CPPlayerState->PointsCaptured;
		}		

		Player->Score += CaptureScore;
	}
}

void AControlPointGameMode::DefaultTimer()
{
	check(Cast<AControlPointGameState>(GameState));

	// Add capture score to controlling team
	if (MatchState == MatchState::InProgress)
	{
		auto CPGameState = CastChecked<AControlPointGameState>(GameState);
		if (auto ControlPoint = CPGameState->GetActiveControlPoint())
		{
			if (ControlPoint->IsCaptured())
			{
				AAtomTeamInfo* ControllingTeam = ControlPoint->GetControllingTeam();
				check(ControllingTeam);

				ControllingTeam->Score += ControlScoreRate;

				if (ControllingTeam->Score >= ScoreLimit)
				{
					CPGameState->SetWinningTeam(ControllingTeam);
				}
			}
		}
	}

	Super::DefaultTimer();
}

void AControlPointGameMode::InitGameStateForRound(AAtomGameState* InGameState)
{
	Super::InitGameStateForRound(InGameState);

	auto ControlPointGameState = CastChecked<AControlPointGameState>(GameState);

	const int32 CurrentRound = ControlPointGameState->CurrentRound;
	const int32 NumControlPoints = GameControlPoints.Num();
	if (NumControlPoints > 0)
	{
		const int32 ObjectiveIndex = (CurrentRound - 1) % NumControlPoints;

		AAtomControlPoint* ControlPoint = GameControlPoints[ObjectiveIndex];
		ControlPointGameState->SetActiveControlPoint(ControlPoint);

		ControlPointCapturedHandle = ControlPoint->OnCaptured().AddUObject(this, &AControlPointGameMode::OnControlPointCaptured);
	}
}

void AControlPointGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	auto ControlPointGameState = CastChecked<AControlPointGameState>(GameState);
	if (AAtomControlPoint* ControlPoint = ControlPointGameState->GetActiveControlPoint())
	{
		ControlPoint->Activate(ObjectiveSpawnDelay);
		BroadcastLocalized(this, ObjectiveMessageClass, 
			UAtomObjectiveMessage::ConstructMessageIndex(UAtomObjectiveMessage::EType::WaitingForObjective, ObjectiveSpawnDelay));
	}
}

void AControlPointGameMode::HandleMatchLeavingIntermission()
{
	// Reset current control point
	auto ControlPointGameState = CastChecked<AControlPointGameState>(GameState);
	if (AAtomControlPoint* ControlPoint = ControlPointGameState->GetActiveControlPoint())
	{	
		ControlPoint->Reset();
	}

	Super::HandleMatchLeavingIntermission();
}

void AControlPointGameMode::HandleMatchEnteredIntermission()
{
	// Deactivate current control point
	auto ControlPointGameState = CastChecked<AControlPointGameState>(GameState);
	if (AAtomControlPoint* ControlPoint = ControlPointGameState->GetActiveControlPoint())
	{
		ControlPoint->Deactivate();
		ControlPoint->OnCaptured().Remove(ControlPointCapturedHandle);
		ControlPointCapturedHandle.Reset();
	}

	Super::HandleMatchEnteredIntermission();
}

void AControlPointGameMode::HandleMatchHasEnded()
{
	// Deactivate current control point
	auto ControlPointGameState = CastChecked<AControlPointGameState>(GameState);
	if (AAtomControlPoint* ControlPoint = ControlPointGameState->GetActiveControlPoint())
	{
		ControlPoint->Deactivate();
		ControlPoint->OnCaptured().Remove(ControlPointCapturedHandle);
		ControlPointCapturedHandle.Reset();
	}

	Super::HandleMatchHasEnded();
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

void AControlPointGameMode::ScoreKill_Implementation(AAtomPlayerState* Killer, AAtomPlayerState* Victim)
{
	// Check if killer or victim is in control point
	auto ControlPointGameState = CastChecked<AControlPointGameState>(GameState);
	AAtomControlPoint* ControlPoint = ControlPointGameState->GetActiveControlPoint();
	auto CPPlayerState = Cast<AControlPointPlayerState>(Killer);

	if (ControlPoint && CPPlayerState)
	{
		UPrimitiveComponent* CaptureBounds = ControlPoint->GetCaptureBounds();

		if (CaptureBounds->IsOverlappingActor(CPPlayerState->GetAtomCharacter()) ||
			CaptureBounds->IsOverlappingActor(Victim->GetAtomCharacter()))
		{
			check(CPPlayerState->GetTeam());

			// Kill occurred on the control point. Determine if it was an attacking or defending kill.
			if (ControlPoint->GetControllingTeam() != CPPlayerState->GetTeam() || !ControlPoint->IsCaptured())
			{
				// Attacking kill
				CPPlayerState->Score += AttackKillScore;
				++CPPlayerState->AttackKills;
			}
			else
			{
				// Defending kill
				CPPlayerState->Score += DefenseKillScore;
				++CPPlayerState->DefenseKills;
			}
		}
	}

	Super::ScoreKill_Implementation(Killer, Victim);
}

void AControlPointGameMode::OnObjectiveInitialized(class AAtomGameObjective* Objective)
{
	Super::OnObjectiveInitialized(Objective);

	if (auto ControlPoint = Cast<AAtomControlPoint>(Objective))
	{
		GameControlPoints.Add(ControlPoint);
	}	
}
