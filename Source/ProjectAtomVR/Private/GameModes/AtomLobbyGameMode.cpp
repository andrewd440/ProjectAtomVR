// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomLobbyGameMode.h"
#include "GameModes/AtomLobbyGameState.h"
#include "AtomGameInstance.h"
#include "AtomPlaylistManager.h"


AAtomLobbyGameMode::AAtomLobbyGameMode()
{
	bUseSeamlessTravel = true;

	GameStateClass = AAtomLobbyGameState::StaticClass();
}

void AAtomLobbyGameMode::InitGameState()
{
	Super::InitGameState();

	if (UGameInstance* const GameInstance = GetGameInstance())
	{
		check(Cast<AAtomLobbyGameState>(GameState));
		check(Cast<UAtomGameInstance>(GameInstance));

		AAtomLobbyGameState* LobbyState = static_cast<AAtomLobbyGameState*>(GameState);
		UAtomGameInstance* AtomGameInstance = static_cast<UAtomGameInstance*>(GameInstance);

		LobbyState->SetNextPlaylistItem(AtomGameInstance->GetPlaylistManager()->CyclePlaylist());
	}
}

bool AAtomLobbyGameMode::ReadyToEndMatch_Implementation()
{
	check(Cast<AAtomLobbyGameState>(GameState));

	// Check for min players for next game
	const AAtomLobbyGameState* const LobbyState = static_cast<AAtomLobbyGameState*>(GameState);
	return NumPlayers >= LobbyState->GetNextPlaylistItem().MinPlayers;
}

void AAtomLobbyGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetMatchState() == MatchState::WaitingPostMatch)
	{
		check(Cast<AAtomLobbyGameState>(GameState));
		const AAtomLobbyGameState* const LobbyState = static_cast<AAtomLobbyGameState*>(GameState);

		if (GetWorld()->GetTimeSeconds() - LobbyState->GetPreGameStartTimeStamp() >= PreGameTimer)
		{
			TravelToNextMatch();
		}
	}
}

void AAtomLobbyGameMode::TravelToNextMatch()
{
	check(Cast<AAtomLobbyGameState>(GameState));

	AAtomLobbyGameState* LobbyState = static_cast<AAtomLobbyGameState*>(GameState);
	const FPlaylistItem& NextGame = LobbyState->GetNextPlaylistItem();

	const FString Url = FString::Printf(TEXT("/Game/Maps/%s?game=%s?listen"), *NextGame.MapName.ToString(), *NextGame.GameMode.ToString());

	GetWorld()->ServerTravel(Url);
}