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
#include "AtomGameObjective.h"

namespace MatchState
{
	const FName Countdown = FName(TEXT("Countdown"));
	const FName Intermission = FName(TEXT("Intermission"));
	const FName ExitingIntermission = FName(TEXT("ExitingIntermission"));
}


AAtomGameMode::AAtomGameMode()
{
	bFirstRoundInitialized = false;
}

void AAtomGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetMatchState() == MatchState::InProgress)
	{
		// Check to see if we should start the match
		if (ReadyToEndRound())
		{
			UE_LOG(LogGameMode, Log, TEXT("GameMode returned ReadyToEndRound"));
			EndRound();
		}
	}
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

	for (TActorIterator<AAtomGameObjective> ObjectiveItr(GetWorld()); ObjectiveItr; ++ObjectiveItr)
	{
		(*ObjectiveItr)->InitializeObjective();
		OnObjectiveInitialized(*ObjectiveItr);
	}
}

void AAtomGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	// Disable moving until match starts
	if (MatchState != MatchState::InProgress)
	{
		NewPlayer->SetCinematicMode(true, false, false, true, false);
	}
}

bool AAtomGameMode::PlayerCanRestart_Implementation(APlayerController* Player)
{
	return MatchState != MatchState::Intermission && 
		MatchState != MatchState::ExitingIntermission && 
		Super::PlayerCanRestart_Implementation(Player);
}

void AAtomGameMode::StartMatch()
{
	if (HasMatchStarted())
	{
		// Already started
		return;
	}

	//Let the game session override the StartMatch function, in case it wants to wait for arbitration
	if (GameSession->HandleStartMatchRequest())
	{
		return;
	}

	// Countdown is always the first state. Initializes systems and players for first round.
	SetMatchState(MatchState::Countdown);		
}

float AAtomGameMode::ModifyDamage_Implementation(float Damage, struct FDamageEvent const& DamageEvent, AController* Inflictor, AController* Reciever) const
{
	return Damage;
}

bool AAtomGameMode::CanDamage_Implementation(AController* Inflictor, AController* Reciever) const
{
	return MatchState == MatchState::InProgress;
}

void AAtomGameMode::InitGameState()
{
	Super::InitGameState();

	AAtomGameState* AtomGameState = GetAtomGameState();
	AtomGameState->ScoreLimit = ScoreLimit;
	AtomGameState->TimeLimit = TimeLimit;
	AtomGameState->Rounds = Rounds;
	AtomGameState->CurrentRound = 1;
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

void AAtomGameMode::OnMatchStateSet()
{
	if (MatchState == MatchState::WaitingToStart)
	{
		HandleMatchIsWaitingToStart();
	}
	else if (MatchState == MatchState::Countdown)
	{
		HandleMatchEnteredCountdown();
	} 
	else if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Intermission)
	{
		HandleMatchEnteredIntermission();
	}
	else if (MatchState == MatchState::ExitingIntermission)
	{
		HandleMatchLeavingIntermission();
	}
	else if (MatchState == MatchState::WaitingPostMatch)
	{
		HandleMatchHasEnded();
	}
	else if (MatchState == MatchState::LeavingMap)
	{
		HandleLeavingMap();
	}
	else if (MatchState == MatchState::Aborted)
	{
		HandleMatchAborted();
	}
}

bool AAtomGameMode::IsCharacterChangeAllowed_Implementation(AAtomPlayerController*) const
{
	return false; // Default to false
}

bool AAtomGameMode::ReadyToEndRound_Implementation()
{
	if (TimeLimit > 0 || ScoreLimit > 0)
	{
		if (AAtomGameState* const AtomGameState = GetGameState<AAtomGameState>())
		{
			return AtomGameState->GetGameWinner() || AtomGameState->RemainingTime < 0;
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

void AAtomGameMode::CheckGameTime()
{
	Super::CheckGameTime();

	if (MatchState != MatchState::InProgress)
	{
		AAtomGameState* AtomGameState = GetAtomGameState();

		if (MatchState == MatchState::Countdown)
		{
			if (AtomGameState->RemainingTime <= 0)
			{
				SetMatchState(MatchState::InProgress);
			}
		}
		else if (MatchState == MatchState::Intermission)
		{
			if (AtomGameState->RemainingTime <= 0)
			{
				SetMatchState(MatchState::ExitingIntermission);
			}
		}		
	}
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
	Rounds = Playlist.Rounds;
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

void AAtomGameMode::InitRound()
{
	AAtomGameState* AtomGameState = GetAtomGameState();

	InitGameStateForRound(AtomGameState);

	for (auto PlayerState : AtomGameState->PlayerArray)
	{
		if (auto AtomPlayerState = Cast<AAtomPlayerState>(PlayerState))
		{
			InitPlayerStateForRound(AtomPlayerState);
		}		
	}
}

void AAtomGameMode::InitGameStateForRound(AAtomGameState* InGameState)
{
	// Nothing for now...
}

void AAtomGameMode::InitPlayerStateForRound(AAtomPlayerState* PlayerState)
{
	// Nothing for now...
}

void AAtomGameMode::OnObjectiveInitialized(class AAtomGameObjective* Objective)
{
	// Nothing for now...
}

void AAtomGameMode::HandleMatchHasStarted()
{
	// Enable pawn movement/input
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		(*Iterator)->SetCinematicMode(false, false, false, true, false);
	}

	// Set match timer
	GetAtomGameState()->RemainingTime = TimeLimit;
}

void AAtomGameMode::HandleMatchEnteredCountdown()
{	
	if (!bFirstRoundInitialized)
	{
		GameSession->HandleMatchHasStarted();
	}

	// start human players first
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = *Iterator;
		if ((PlayerController->GetPawn() == nullptr) && PlayerCanRestart(PlayerController))
		{
			RestartPlayer(PlayerController);
		}
	}

	InitRound();

	if (!bFirstRoundInitialized)
	{
		// Make sure level streaming is up to date before triggering NotifyMatchStarted
		GEngine->BlockTillLevelStreamingCompleted(GetWorld());

		// First fire BeginPlay, if we haven't already in waiting to start match
		GetWorldSettings()->NotifyBeginPlay();

		// Then fire off match started
		GetWorldSettings()->NotifyMatchStarted();
	}

	// if passed in bug info, send player to right location
	const FString BugLocString = UGameplayStatics::ParseOption(OptionsString, TEXT("BugLoc"));
	const FString BugRotString = UGameplayStatics::ParseOption(OptionsString, TEXT("BugRot"));
	if (!BugLocString.IsEmpty() || !BugRotString.IsEmpty())
	{
		for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			APlayerController* PlayerController = *Iterator;
			if (PlayerController->CheatManager != nullptr)
			{
				PlayerController->CheatManager->BugItGoString(BugLocString, BugRotString);
			}
		}
	}

	if (!bFirstRoundInitialized && IsHandlingReplays() && GetGameInstance() != nullptr)
	{
		GetGameInstance()->StartRecordingReplay(GetWorld()->GetMapName(), GetWorld()->GetMapName());
	}	

	GetAtomGameState()->RemainingTime = CountdownTime;
	bFirstRoundInitialized = true;
}

bool AAtomGameMode::IsMatchFinished() const
{
	return GetAtomGameState()->CurrentRound >= Rounds;
}

void AAtomGameMode::EndRound()
{
	if (IsMatchFinished())
	{
		EndMatch();
	}
	else
	{		
		SetMatchState(MatchState::Intermission);
	}
}

void AAtomGameMode::HandleMatchEnteredIntermission()
{
	// Disable pawn movement input
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		(*Iterator)->SetCinematicMode(true, false, false, true, false);
	}

	GetAtomGameState()->RemainingTime = IntermissionTime;
}

void AAtomGameMode::HandleMatchLeavingIntermission()
{
	// Destroy pawns. They will be recreated next round.
	// #AtomTodo Iterating controller list to destroy pawns. Investigate pawns not added to world for remotes for some reason.
	for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
	{
		AController* Controller = *Iterator;
		if (Controller->GetPawn())
		{
			Controller->GetPawn()->Destroy();
		}
	}

	++GetAtomGameState()->CurrentRound;
	SetMatchState(MatchState::Countdown);
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

bool AAtomGameMode::IsMatchInProgress() const
{
	return MatchState == MatchState::Countdown || 
		MatchState == MatchState::Intermission || 
		MatchState == MatchState::ExitingIntermission || 
		Super::IsMatchInProgress();
}

