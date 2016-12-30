// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomGameState.h"

#include "AtomPlayerState.h"


AAtomGameState::AAtomGameState()
{

}

void AAtomGameState::ScoreKill(AAtomPlayerState* Player, int32 Score)
{
	++Player->Kills;
	Player->Score += Score;

	check(Player->TeamId >= 0);
	Teams[Player->TeamId].Score += Score;
}

void AAtomGameState::ScoreDeath(AAtomPlayerState* Player, int32 Score)
{
	++Player->Deaths;
	Player->Score += Score;

	check(Player->TeamId >= 0);
	Teams[Player->TeamId].Score += Score;
}

void AAtomGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	// Assign team and add to list
	if (AAtomPlayerState* const AtomPlayerState = Cast<AAtomPlayerState>(PlayerState))
	{
		if (AtomPlayerState->TeamId < 0)
		{
			AtomPlayerState->TeamId = (Teams[0].Players.Num() < Teams[1].Players.Num()) ? 0 : 1;
		}

		Teams[AtomPlayerState->TeamId].Players.AddUnique(AtomPlayerState);
	}
}

void AAtomGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);

	// Remove from assigned team
	if (AAtomPlayerState* const AtomPlayerState = Cast<AAtomPlayerState>(PlayerState))
	{
		if (AtomPlayerState->TeamId >= 0)
		{
			Teams[AtomPlayerState->TeamId].Players.Remove(AtomPlayerState);
			// Keep assigned team in player state while it persists in InactivePlayerArray in GameMode			
		}
	}
}

void AAtomGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAtomGameState, Teams);
}
