// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomTeamGameMode.h"
#include "AtomTeamInfo.h"
#include "Color.h"
#include "AtomPlayerState.h"


DEFINE_LOG_CATEGORY_STATIC(LogAtomTeamGameMode, Log, All);

AAtomTeamGameMode::AAtomTeamGameMode()
{
	new(TeamColors) FLinearColor(FLinearColor::Red);
	new(TeamColors) FLinearColor(FLinearColor::Blue);
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

	if (bNoGameplayMute)
		return;

	// Mute opposing teams and unmute team
	AAtomPlayerState* const AtomPlayerState = Cast<AAtomPlayerState>(aPlayer->PlayerState);
	
	int32 TeamId = -1;
	if (AtomPlayerState && AtomPlayerState->GetTeam())
	{
		TeamId = AtomPlayerState->GetTeam()->TeamId;
	}

	const auto& PlayerNetId = aPlayer->NetConnection->PlayerId;
	
	auto& Teams = GetAtomGameState()->Teams;
	for (int32 i = 0; i < Teams.Num(); ++i)
	{
		if (i != TeamId)
		{
			for (auto& Controller : Teams[i]->GetTeamMembers())
			{
				if (auto PlayerController = Cast<APlayerController>(Controller))
				{
					if (PlayerController->NetConnection)
					{
						aPlayer->GameplayMutePlayer(PlayerController->NetConnection->PlayerId);
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
					if (PlayerController->NetConnection)
					{
						aPlayer->GameplayUnmutePlayer(PlayerController->NetConnection->PlayerId);
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

void AAtomTeamGameMode::GenericPlayerInitialization(AController* C)
{
	if (auto PlayerState = Cast<AAtomPlayerState>(C->PlayerState))
	{
		auto& Teams = GetAtomGameState()->Teams;

		// Assign team if not assigned
		if (Teams.IsValidIndex(PlayerState->GetSavedTeamId()))
		{
			PlayerState->SetTeam(Teams[PlayerState->GetSavedTeamId()]);
		}
		else
		{			
			PlayerState->SetTeam(ChooseBestTeam(C));
		}
		
		PlayerState->GetTeam()->AddToTeam(C);
	}
	else
	{
		UE_LOG(LogAtomTeamGameMode, Warning, TEXT("Incoming player without valid AtomPlayerState."));
	}

	// Call after setting up team to update gameplay mute list correctly
	Super::GenericPlayerInitialization(C);
}
