// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomLobbyGameMode.h"
#include "GameModes/AtomLobbyGameState.h"
#include "AtomGameInstance.h"
#include "AtomPlaylistManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogLobbyGameMode, Log, All);

AAtomLobbyGameMode::AAtomLobbyGameMode()
{
	bUseSeamlessTravel = true;

	bMuteTeams = false;
	bDelayCharacterLoadoutCreation = true;

	GameStateClass = AAtomLobbyGameState::StaticClass();
}

void AAtomLobbyGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	check(!UGameplayStatics::GetIntOption(Options, TEXT("bUsePlaylist"), 0) != 0 && "Playlist should not be used in lobby settings.");

	if (UGameInstance* const GameInstance = GetGameInstance())
	{
		check(Cast<UAtomGameInstance>(GameInstance));

		UAtomGameInstance* AtomGameInstance = CastChecked<UAtomGameInstance>(GameInstance);
		const FPlaylistItem& NextMatch = AtomGameInstance->GetPlaylistManager()->CyclePlaylist();

		MinPlayers = NextMatch.MinPlayers;
		MaxPlayers = NextMatch.MaxPlayers;
		TeamCount = NextMatch.TeamCount;
		TeamColors = NextMatch.TeamColors;
	}

	// Never any rounds or countdown for lobby
	Rounds = 1;
	CountdownTime = 0;
}

void AAtomLobbyGameMode::InitGameState()
{
	Super::InitGameState();

	if (UGameInstance* const GameInstance = GetGameInstance())
	{
		AAtomLobbyGameState* LobbyState = CastChecked<AAtomLobbyGameState>(GameState);
		UAtomGameInstance* AtomGameInstance = CastChecked<UAtomGameInstance>(GameInstance);

		LobbyState->SetNextPlaylistItem(AtomGameInstance->GetPlaylistManager()->CurrentItem());
	}
}

void AAtomLobbyGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetMatchState() == MatchState::InProgress)
	{
		check(Cast<AAtomLobbyGameState>(GameState));
		AAtomLobbyGameState* const LobbyState = CastChecked<AAtomLobbyGameState>(GameState);

		const bool bHasMinPlayers = LobbyState->PlayerArray.Num() >= MinPlayers;
		const bool bPreGameStarted = LobbyState->GetPreGameStartTimeStamp() != 0;

		if (!bHasMinPlayers && bPreGameStarted)
		{
			LobbyState->SetPreGameStartTimeStamp(0);
		}
		else if (bHasMinPlayers)
		{
			if (!bPreGameStarted)
			{
				LobbyState->SetPreGameStartTimeStamp(GetWorld()->GetTimeSeconds());
			}
			else if (GetWorld()->GetTimeSeconds() - LobbyState->GetPreGameStartTimeStamp() >= PreGameTimer)
			{
				EndMatch();
			}
		}
	}
}

bool AAtomLobbyGameMode::CanDamage_Implementation(AController* Inflictor, AController* Reciever) const
{
	return false; // No damage in lobby
}

void AAtomLobbyGameMode::TravelToNextMatch()
{
	check(Cast<AAtomLobbyGameState>(GameState));

	AAtomLobbyGameState* LobbyState = CastChecked<AAtomLobbyGameState>(GameState);
	const FPlaylistItem& NextGame = LobbyState->GetNextPlaylistItem();

	const FString Url = FString::Printf(TEXT("/Game/Maps/%s?game=%s?listen?bUsePlaylist=1"), *NextGame.MapName.ToString(), *NextGame.GameMode.ToString());

	GetWorld()->ServerTravel(Url);
}

bool AAtomLobbyGameMode::IsCharacterChangeAllowed_Implementation(class AAtomPlayerController* Controller) const
{
	return true; // Always allow change in lobby
}
