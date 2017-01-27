// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomTeamGameMode.h"
#include "AtomTeamInfo.h"
#include "Color.h"
#include "AtomPlayerState.h"
#include "AtomTeamStart.h"


DEFINE_LOG_CATEGORY_STATIC(LogAtomTeamGameMode, Log, All);

AAtomTeamGameMode::AAtomTeamGameMode()
{
	new(TeamColors) FLinearColor(FLinearColor::Red);
	new(TeamColors) FLinearColor(FLinearColor::Blue);

	bBalanceTeams = true;
	bMuteTeams = true;
}

bool AAtomTeamGameMode::ChangeTeams(AController* Controller, int32 TeamId)
{
	AAtomGameState* AtomGameState = GetAtomGameState();
	check(AtomGameState->Teams.IsValidIndex(TeamId));

	check(Cast<AAtomPlayerState>(Controller->PlayerState));
	AAtomPlayerState* PlayerState = static_cast<AAtomPlayerState*>(Controller->PlayerState);
	
	AAtomTeamInfo* OldTeam = PlayerState->GetTeam();
	AAtomTeamInfo* NewTeam = AtomGameState->Teams[TeamId];

	// If new team has less members, allow switch
	if (!bBalanceTeams || NewTeam->Size() < OldTeam->Size())
	{
		MovePlayerToTeam(Controller, PlayerState, NewTeam);
		return true;
	}

	// Check if any members are waiting to switch teams
	for (auto TeamMember : NewTeam->GetTeamMembers())
	{
		check(Cast<AAtomPlayerState>(TeamMember->PlayerState));
		AAtomPlayerState* TeamMemberState = static_cast<AAtomPlayerState*>(TeamMember->PlayerState);

		if (TeamMemberState->GetPendingTeamChange() == OldTeam->TeamId)
		{
			MovePlayerToTeam(Controller, PlayerState, NewTeam);
			MovePlayerToTeam(TeamMember, TeamMemberState, OldTeam);
			return true;
		}
	}

	// Team change rejected. Set pending and return.
	PlayerState->SetPendingTeamChange(TeamId);
	return false;
}

void AAtomTeamGameMode::GetSeamlessTravelActorList(bool bToTransition, TArray<AActor *>& ActorList)
{
	Super::GetSeamlessTravelActorList(bToTransition, ActorList);

	// Save teams when traveling to transition level. When the final level is loaded, new teams will be created.
	if (bToTransition)
	{
		const TArray<AAtomTeamInfo*> Teams = GetAtomGameState()->Teams;
		for (auto Team : Teams)
		{
			ActorList.Add(Team);
		}
	}
}

void AAtomTeamGameMode::MovePlayerToTeam(AController* Controller, AAtomPlayerState* PlayerState, AAtomTeamInfo* Team)
{
	// Unposses and destroy existing pawn
	if (APawn* Pawn = Controller->GetPawn())
	{
		Pawn->DetachFromControllerPendingDestroy();
		Pawn->Destroy(true);
	}

	// Remove from existing team
	if (PlayerState->GetTeam())
	{
		PlayerState->GetTeam()->RemoveFromTeam(Controller);
	}

	PlayerState->SetPendingTeamChange(-1);
	
	// Add to new team
	PlayerState->SetTeam(Team);
	Team->AddToTeam(Controller);

	if (auto PlayerController = Cast<APlayerController>(Controller))
	{
		UpdateGameplayMuteList(PlayerController);
	}	

	RestartPlayer(Controller);
}

bool AAtomTeamGameMode::CanDamage_Implementation(AController* Inflictor, AController* Reciever) const
{
	AAtomPlayerState* InflictorState = Inflictor ? Cast<AAtomPlayerState>(Inflictor->PlayerState) : nullptr;
	AAtomPlayerState* RecieverState = Reciever ? Cast<AAtomPlayerState>(Reciever->PlayerState) : nullptr;

	if (InflictorState && RecieverState && InflictorState->GetTeam() == RecieverState->GetTeam())
	{
		return false;
	}
	
	return true;
}

void AAtomTeamGameMode::Logout(AController* Exiting)
{	
	if (auto PlayerState = Cast<AAtomPlayerState>(Exiting->PlayerState))
	{
		// Remove from team if not assigned
		if (AAtomTeamInfo* Team = PlayerState->GetTeam())
		{
			Team->RemoveFromTeam(Exiting);
		}
	}

	Super::Logout(Exiting);
}

AAtomTeamInfo* AAtomTeamGameMode::ChooseBestTeam(const AController* Controller) const
{
	const TArray<AAtomTeamInfo*>& Teams = GetAtomGameState()->Teams;

	// Get teams with the lowest player count
	TArray<AAtomTeamInfo*, TInlineAllocator<2>> LowestPlayerCounts;
	int32 LowestPlayerCount = MaxPlayers;
	for (AAtomTeamInfo* Team : Teams)
	{
		const int32 TeamSize = Team->Size();

		if (TeamSize == LowestPlayerCount)
		{
			LowestPlayerCounts.Add(Team);
		}
		else if (TeamSize < LowestPlayerCount)
		{
			LowestPlayerCounts.Empty(1);
			LowestPlayerCounts.Add(Team);
			LowestPlayerCount = TeamSize;
		}
	}

	// Use team with lowest score and player count
	AAtomTeamInfo* BestTeam = LowestPlayerCounts[0];
	for (int32 i = 1; i < LowestPlayerCounts.Num(); ++i)
	{
		if (LowestPlayerCounts[i]->Score < BestTeam->Score)
		{
			BestTeam = LowestPlayerCounts[i];
		}
	}

	return BestTeam;
}

void AAtomTeamGameMode::UpdateGameplayMuteList(APlayerController* aPlayer)
{
	Super::UpdateGameplayMuteList(aPlayer);
	
	static auto NoMuteCmdInit = []() 
	{
		TArray<FString> Tokens, Switches;
		FCommandLine::Parse(FCommandLine::Get(), Tokens, Switches);
		return Switches.Find(TEXT("NoGameplayMute")) != INDEX_NONE;
	};

	static bool bNoGameplayMute = NoMuteCmdInit();

	if (bNoGameplayMute || !bMuteTeams)
		return;

	// Mute opposing teams and unmute team
	AAtomPlayerState* const AtomPlayerState = Cast<AAtomPlayerState>(aPlayer->PlayerState);
	
	int32 TeamId = AAtomTeamInfo::INDEX_NO_TEAM;
	if (AtomPlayerState && AtomPlayerState->GetTeam())
	{
		TeamId = AtomPlayerState->GetTeam()->TeamId;
	}

	const auto& PlayerNetId = aPlayer->PlayerState->UniqueId;
	
	auto& Teams = GetAtomGameState()->Teams;
	for (int32 i = 0; i < Teams.Num(); ++i)
	{
		if (i != TeamId)
		{
			for (auto& Controller : Teams[i]->GetTeamMembers())
			{				
				if (auto PlayerController = Cast<APlayerController>(Controller))
				{
					const auto& OtherNetId = PlayerController->PlayerState->UniqueId;
					if (OtherNetId.IsValid())
					{
						aPlayer->GameplayMutePlayer(OtherNetId);
						PlayerController->GameplayMutePlayer(PlayerNetId);
					}
				}

			}
		}
		else
		{
			for (auto& Controller : Teams[i]->GetTeamMembers())
			{
				if (Controller == aPlayer)
					continue;

				if (auto PlayerController = Cast<APlayerController>(Controller))
				{
					const auto& OtherNetId = PlayerController->PlayerState->UniqueId;
					if (OtherNetId.IsValid())
					{
						aPlayer->GameplayUnmutePlayer(OtherNetId);
						PlayerController->GameplayUnmutePlayer(PlayerNetId);
					}
				}
			}
		}
	}
}

void AAtomTeamGameMode::InitGameState()
{
	Super::InitGameState();

	// Setup teams
	AAtomGameState* AtomGameState = GetAtomGameState();
	AtomGameState->bIsTeamGame = true;

	AtomGameState->Teams.SetNum(TeamCount);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.ObjectFlags |= RF_Transient;

	for (int32 i = 0; i < TeamCount; ++i)
	{
		AAtomTeamInfo* NewTeam = GetWorld()->SpawnActor<AAtomTeamInfo>(SpawnParams);

		if (TeamColors.IsValidIndex(i))
		{
			NewTeam->TeamColor = TeamColors[i];
		}

		NewTeam->TeamId = i;
		AtomGameState->Teams[i] = NewTeam;
	}
}

void AAtomTeamGameMode::ApplyPlaylistSettings(const FPlaylistItem& Playlist)
{
	TeamCount = Playlist.TeamCount;
	TeamColors = Playlist.TeamColors;
}

bool AAtomTeamGameMode::IsValidPlayerStart(AController* Player, APlayerStart* PlayerStart)
{
	AAtomPlayerState* AtomPlayerState = Cast<AAtomPlayerState>(Player->PlayerState);
	AAtomTeamStart* AtomTeamStart = Cast<AAtomTeamStart>(PlayerStart);

	return !AtomPlayerState || (AtomTeamStart && (AtomPlayerState->GetTeam()->TeamId == AtomTeamStart->TeamId));
}

void AAtomTeamGameMode::CheckForGameWinner_Implementation(AAtomPlayerState* Scorer)
{
	if (ScoreLimit <= 0)
		return;

	AAtomGameState* AtomGameState = GetAtomGameState();
	AAtomTeamInfo* ScoringTeam = Scorer->GetTeam();
	
	if (ScoringTeam->Score >= ScoreLimit)
	{
		AtomGameState->SetGameWinner(Scorer);
	}
}

FString AAtomTeamGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal)
{
	// Assign team before Super for things that rely on teams (mute list, player start)
	if (auto PlayerState = Cast<AAtomPlayerState>(NewPlayerController->PlayerState))
	{
		AAtomTeamInfo* Team = ChooseBestTeam(NewPlayerController);
		PlayerState->SetTeam(Team);
		Team->AddToTeam(NewPlayerController);
	}

	return Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);
}

void AAtomTeamGameMode::InitSeamlessTravelPlayer(AController* NewController)
{
	// Assign team before Super for things that rely on teams (mute list, player start)
	if (auto PlayerState = Cast<AAtomPlayerState>(NewController->PlayerState))
	{
		auto& Teams = GetAtomGameState()->Teams;

		// Assign team if not assigned
		if (Teams.IsValidIndex(PlayerState->GetSavedTeamId()))
		{
			PlayerState->SetTeam(Teams[PlayerState->GetSavedTeamId()]);
		}
		else
		{
			PlayerState->SetTeam(ChooseBestTeam(NewController));
		}

		PlayerState->GetTeam()->AddToTeam(NewController);
	}

	Super::InitSeamlessTravelPlayer(NewController);
}
