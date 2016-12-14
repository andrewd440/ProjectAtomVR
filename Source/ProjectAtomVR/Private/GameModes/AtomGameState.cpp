// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomGameState.h"

#include "AtomPlayerState.h"


AAtomGameState::AAtomGameState()
{

}

void AAtomGameState::AddTeamScore(int TeamId, int Score)
{
	check(TeamId < 2 && TeamId >= 0);

	Teams[TeamId].Score += Score;
}

void AAtomGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	// Assign team and add to list
	if (AAtomPlayerState* const AtomPlayerState = Cast<AAtomPlayerState>(PlayerState))
	{
		if (AtomPlayerState->GetAssignedTeam() < 0)
		{
			AtomPlayerState->AssignTeam(Teams[0].Num() < Teams[1].Num() ? 0 : 1);
		}

		Teams[AtomPlayerState->GetAssignedTeam()].AddUnique(AtomPlayerState);
	}
}

void AAtomGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);

	// Remove from assigned team
	if (AAtomPlayerState* const AtomPlayerState = Cast<AAtomPlayerState>(PlayerState))
	{
		const int32 AssignedTeam = AtomPlayerState->GetAssignedTeam();

		if (AssignedTeam >= 0)
		{
			Teams[AtomPlayerState->GetAssignedTeam()].Remove(AtomPlayerState);
			// Keep assigned team in player state while it persists in InactivePlayerArray in GameMode			
		}
	}
}
