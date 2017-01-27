// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomGameMode.h"
#include "AtomPlayerController.h"
#include "AtomCharacter.h"
#include "AtomGameInstance.h"
#include "AtomTeamStart.h"
#include "Engine/World.h"
#include "AtomPlayerState.h"
#include "AtomGameState.h"
#include "AtomTeamInfo.h"
#include "Engine/EngineTypes.h"
#include "AtomPlaylistManager.h"

AAtomGameMode::AAtomGameMode()
{

}

void AAtomGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	if (UGameplayStatics::GetIntOption(Options, TEXT("bUsePlaylist"), 0) != 0)
	{
		if (UGameInstance* const GameInstance = GetGameInstance())
		{
			check(Cast<UAtomGameInstance>(GameInstance));

			UAtomGameInstance* AtomGameInstance = static_cast<UAtomGameInstance*>(GameInstance);
			const FPlaylistItem& Playlist = AtomGameInstance->GetPlaylistManager()->CurrentItem();

			ApplyPlaylistSettings(Playlist);
		}
	}
}

float AAtomGameMode::ModifyDamage_Implementation(float Damage, struct FDamageEvent const& DamageEvent, AController* Inflictor, AController* Reciever) const
{
	return Damage;
}

bool AAtomGameMode::CanDamage_Implementation(AController* Inflictor, AController* Reciever) const
{
	return true;
}

void AAtomGameMode::InitGameState()
{
	Super::InitGameState();

	AAtomGameState* AtomGameState = GetAtomGameState();
	AtomGameState->ScoreLimit = ScoreLimit;
	AtomGameState->TimeLimit = TimeLimit;
}

void AAtomGameMode::ScoreKill_Implementation(AController* Killer, AController* Victim)
{
	const bool bIsSuicide = (Killer == Victim);

	AAtomGameState* const AtomGameState = GetAtomGameState();

	// Score kill
	if (!bIsSuicide)
	{
		if (AAtomPlayerState* KillerState = Cast<AAtomPlayerState>(Killer->PlayerState))
		{
			KillerState->Score += KillScore;
			++KillerState->Kills;

			if (AtomGameState->bIsTeamGame)
				KillerState->GetTeam()->Score += KillScore;

			CheckForGameWinner(KillerState);
		}
	}

	// Score death/suicide
	if (AAtomPlayerState* VictimState = Cast<AAtomPlayerState>(Victim->PlayerState))
	{
		VictimState->Score += DeathScore;
		++VictimState->Deaths;

		if(AtomGameState->bIsTeamGame)
			VictimState->GetTeam()->Score += DeathScore;
	}	
}

bool AAtomGameMode::IsCharacterChangeAllowed_Implementation(AAtomPlayerController*) const
{
	return false; // Default to false
}

bool AAtomGameMode::ReadyToEndMatch_Implementation()
{
	if (Super::ReadyToEndMatch_Implementation())
	{
		return true;
	}
	else if (TimeLimit > 0 || ScoreLimit > 0)
	{
		if (AAtomGameState* const AtomGameState = GetGameState<AAtomGameState>())
		{
			return AtomGameState->GetGameWinner() || AtomGameState->ElapsedTime >= TimeLimit;
		}		
	}

	return false;
}

void AAtomGameMode::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();

	TravelToNextMatch();
}

bool AAtomGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
	return false;
}

bool AAtomGameMode::IsValidPlayerStart(AController*, APlayerStart*)
{
	return true;
}

void AAtomGameMode::ApplyPlaylistSettings(const FPlaylistItem& Playlist)
{
	MinPlayers = Playlist.MinPlayers;
	MaxPlayers = Playlist.MaxPlayers;
	TimeLimit = Playlist.TimeLimit;
	ScoreLimit = Playlist.ScoreLimit;
}

void AAtomGameMode::TravelToNextMatch()
{
	// Travel back to lobby by default
	check(Cast<UAtomGameInstance>(GetGameInstance()));

	UAtomGameInstance* const GameInstance = static_cast<UAtomGameInstance*>(GetGameInstance());

	const FString URLString = FString::Printf(TEXT("/Game/Maps/%s?listen?game=%s?bUsePlaylist=0"), *GameInstance->GetLobbyMap().ToString(),
		*GameInstance->GetLobbyGameMode().ToString());
	GetWorld()->ServerTravel(URLString);
}

void AAtomGameMode::CheckForGameWinner_Implementation(AAtomPlayerState* Scorer)
{
	if (ScoreLimit > 0)
	{
		if (Scorer->Score > ScoreLimit)
		{
			GetAtomGameState()->SetGameWinner(Scorer);
		}
	}
}

AActor* AAtomGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	// Choose a player start
	APlayerStart* FoundPlayerStart = nullptr;
	UClass* PawnClass = GetDefaultPawnClassForController(Player);
	APawn* PawnToFit = PawnClass ? PawnClass->GetDefaultObject<APawn>() : nullptr;

	TArray<APlayerStart*> UnOccupiedStartPoints;
	TArray<APlayerStart*> OccupiedStartPoints;
	
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* PlayerStart = *It;

		if (PlayerStart->IsA<APlayerStartPIE>())
		{
			// Always prefer the first "Play from Here" PlayerStart, if we find one while in PIE mode
			FoundPlayerStart = PlayerStart;
			break;
		}
		else if(IsValidPlayerStart(Player, PlayerStart))
		{
			FVector ActorLocation = PlayerStart->GetActorLocation();
			const FRotator ActorRotation = PlayerStart->GetActorRotation();
			if (!GetWorld()->EncroachingBlockingGeometry(PawnToFit, ActorLocation, ActorRotation))
			{
				UnOccupiedStartPoints.Add(PlayerStart);
			}
			else if (GetWorld()->FindTeleportSpot(PawnToFit, ActorLocation, ActorRotation))
			{
				OccupiedStartPoints.Add(PlayerStart);
			}
		}
	}

	if (FoundPlayerStart == nullptr)
	{
		if (UnOccupiedStartPoints.Num() > 0)
		{
			FoundPlayerStart = UnOccupiedStartPoints[FMath::RandRange(0, UnOccupiedStartPoints.Num() - 1)];
		}
		else if (OccupiedStartPoints.Num() > 0)
		{
			FoundPlayerStart = OccupiedStartPoints[FMath::RandRange(0, OccupiedStartPoints.Num() - 1)];
		}
	}

	return FoundPlayerStart != nullptr ? FoundPlayerStart : Super::ChoosePlayerStart_Implementation(Player);
}

