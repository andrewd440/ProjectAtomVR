// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomPlayerState.h"
#include "AtomPlayerController.h"
#include "AtomCharacter.h"
#include "AtomTeamInfo.h"
#include "Engine/World.h"


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

int32 AAtomPlayerState::GetSavedTeamId() const
{
	return SavedTeamId;
}

void AAtomPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AAtomPlayerState, Team);
	DOREPLIFETIME(AAtomPlayerState, Kills);
	DOREPLIFETIME(AAtomPlayerState, Deaths);
}

void AAtomPlayerState::ClientInitialize(class AController* C)
{
	Super::ClientInitialize(C);

	AtomCharacter = Cast<AAtomCharacter>(C->GetPawn());
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
