// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomLobbyGameState.h"




AAtomLobbyGameState::AAtomLobbyGameState()
{

}

void AAtomLobbyGameState::SetNextPlaylistItem(const FPlaylistItem& Item)
{
	NextPlaylistItem = Item;
}

const FPlaylistItem& AAtomLobbyGameState::GetNextPlaylistItem() const
{
	return NextPlaylistItem;
}

void AAtomLobbyGameState::OnRep_NextPlaylistItem()
{

}

void AAtomLobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAtomLobbyGameState, NextPlaylistItem);
	DOREPLIFETIME(AAtomLobbyGameState, PreGameStartTimeStamp);
}
