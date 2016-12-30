// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomPlayerState.h"


void AAtomPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAtomPlayerState, TeamId);
	DOREPLIFETIME(AAtomPlayerState, Kills);
	DOREPLIFETIME(AAtomPlayerState, Deaths);
}

void AAtomPlayerState::OnRep_TeamId()
{

}
