// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomGameState.h"

#include "AtomPlayerState.h"


AAtomGameState::AAtomGameState()
{

}

void AAtomGameState::SetGameWinner(AAtomPlayerState* Winner)
{
	GameWinner = Winner;
}

AAtomPlayerState* AAtomGameState::GetGameWinner() const
{
	return GameWinner;
}

void AAtomGameState::DefaultTimer()
{
	if (MatchState == MatchState::InProgress || MatchState == MatchState::Intermission || MatchState == MatchState::Countdown)
	{
		--RemainingTime;
	}

	Super::DefaultTimer();
}

void AAtomGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAtomGameState, Teams);
	DOREPLIFETIME(AAtomGameState, GameWinner);
	DOREPLIFETIME(AAtomGameState, CurrentRound);
	DOREPLIFETIME(AAtomGameState, RemainingTime);

	DOREPLIFETIME_CONDITION(AAtomGameState, bIsTeamGame, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(AAtomGameState, ScoreLimit, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(AAtomGameState, TimeLimit, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(AAtomGameState, Rounds, COND_InitialOnly);
}