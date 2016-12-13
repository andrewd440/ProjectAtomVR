// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomPlayerState.h"




void AAtomPlayerState::AssignTeam(int32 Id)
{
	TeamId = Id;
}

int32 AAtomPlayerState::GetAssignedTeam() const
{
	return TeamId;
}

void AAtomPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAtomPlayerState, TeamId);
}

void AAtomPlayerState::OnRep_TeamId()
{

}
