// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomPlayerState.h"
#include "AtomPlayerController.h"
#include "AtomCharacter.h"
#include "AtomTeamInfo.h"
#include "Engine/World.h"
#include "AtomGameState.h"
#include "AtomTeamGameMode.h"


AAtomPlayerState::AAtomPlayerState()
{
	bReplicates = true;
}

AAtomTeamInfo* AAtomPlayerState::GetTeam() const
{
	return Team;
}

void AAtomPlayerState::SetTeam(AAtomTeamInfo* InTeam)
{
	Team = InTeam;
	SavedTeamId = InTeam->TeamId;
	NotifyTeamChanged();
}

uint32 AAtomPlayerState::GetSavedTeamId() const
{
	return SavedTeamId;
}

void AAtomPlayerState::SetPendingTeamChange(uint32 InTeamId)
{
	PendingTeamChange = static_cast<uint8>(InTeamId);
}

uint32 AAtomPlayerState::GetPendingTeamChange() const
{
	return PendingTeamChange;
}

void AAtomPlayerState::ServerRequestTeamChange_Implementation(int32 RequestedTeam)
{
	AController* Controller = Cast<AController>(GetOwner());
	AAtomTeamGameMode* GameMode = GetWorld()->GetAuthGameMode<AAtomTeamGameMode>();
	
	if (Controller && GameMode)
	{
		AAtomGameState* GameState = CastChecked<AAtomGameState>(GameMode->GameState);

		if (RequestedTeam == AAtomTeamInfo::INDEX_NO_TEAM)
		{			
			RequestedTeam = (Team != nullptr && GameState->Teams.IsValidIndex(Team->TeamId + 1)) ? Team->TeamId + 1 : 0;	
		}

		GameMode->ChangeTeams(Controller, RequestedTeam);
	}
}

bool AAtomPlayerState::ServerRequestTeamChange_Validate(int32)
{
	return true;
}

void AAtomPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AAtomPlayerState, Team);
	DOREPLIFETIME(AAtomPlayerState, Kills);
	DOREPLIFETIME(AAtomPlayerState, Deaths);
	DOREPLIFETIME(AAtomPlayerState, PendingTeamChange);
}

void AAtomPlayerState::ClientInitialize(class AController* C)
{
	Super::ClientInitialize(C);

	AtomCharacter = Cast<AAtomCharacter>(C->GetPawn());
}

void AAtomPlayerState::Reset()
{
	Super::Reset();

	Kills = 0;
	Deaths = 0;
	PendingTeamChange = AAtomTeamInfo::INDEX_NO_TEAM;
}

void AAtomPlayerState::NotifyTeamChanged()
{
	if (AAtomCharacter* Character = GetAtomCharacter())
	{
		Character->NotifyTeamChanged();
	}
}

AAtomCharacter* AAtomPlayerState::GetAtomCharacter() const
{
	if (AtomCharacter && !AtomCharacter->IsPendingKill() && AtomCharacter->PlayerState == this)
	{
		return AtomCharacter;
	}

	if (AController* C = Cast<AController>(GetOwner()))
	{
		AtomCharacter = Cast<AAtomCharacter>(C->GetPawn());
		return AtomCharacter;
	}

	// Iterate all pawns and search for matching playerstate
	for (FConstPawnIterator Iterator = GetWorld()->GetPawnIterator(); Iterator; ++Iterator)
	{
		const TAutoWeakObjectPtr<APawn> Pawn = *Iterator;
		if (Pawn.IsValid() && Pawn->PlayerState == this && !Pawn->IsPendingKill())
		{
			AtomCharacter = Cast<AAtomCharacter>(Pawn.Get());
			return AtomCharacter;
		}
	}

	return nullptr;
}

void AAtomPlayerState::OnRep_PendingTeamChange()
{
	// Do nothing yet...
}

void AAtomPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	if (auto AtomPlayerState = Cast<AAtomPlayerState>(PlayerState))
	{
		AtomPlayerState->SavedTeamId = SavedTeamId;
	}
}
