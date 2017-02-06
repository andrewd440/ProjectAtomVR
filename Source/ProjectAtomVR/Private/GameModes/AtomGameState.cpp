// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomGameState.h"

#include "AtomPlayerState.h"
#include "AtomTeamInfo.h"

#define LOCTEXT_NAMESPACE "AtomGameState"

AAtomGameState::AAtomGameState()
{

}

void AAtomGameState::SetWinningTeam(AAtomTeamInfo* Team)
{
	 auto WinningPlayer = Cast<AAtomPlayerState>(Team->GetTeamMembers()[0]->PlayerState);
	 SetGameWinner(WinningPlayer);
}

void AAtomGameState::SetGameWinner(AAtomPlayerState* Winner)
{
	GameWinner = Winner;
}

AAtomPlayerState* AAtomGameState::GetGameWinner() const
{
	return GameWinner;
}

FText AAtomGameState::GetGameStatusText() const
{
	if (!IsMatchInProgress())
	{
		if (MatchState == MatchState::Countdown)
		{
			return FText::Format(LOCTEXT("GameStatusCountdown", "Countdown: {0}"), FText::AsNumber(RemainingTime));
		}
		else if (MatchState == MatchState::Intermission)
		{
			return FText::Format(LOCTEXT("GameStatusIntermission", "Intermission: {0}"), FText::AsNumber(RemainingTime));
		}
	}

	return FText::GetEmpty();
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

#undef LOCTEXT_NAMESPACE