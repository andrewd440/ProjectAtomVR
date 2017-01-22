// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomTeamInfo.h"



AAtomTeamInfo::AAtomTeamInfo()
{
	bReplicates = true;
	bAlwaysRelevant = true;
	bReplicateMovement = false;
	NetUpdateFrequency = 1;
}

void AAtomTeamInfo::AddToTeam(AController* Controller)
{
	TeamMembers.Add(Controller);
}

void AAtomTeamInfo::RemoveFromTeam(AController* Controller)
{
	TeamMembers.Remove(Controller);
}

const TArray<AController*>& AAtomTeamInfo::GetTeamMembers() const
{
	return TeamMembers;
}

void AAtomTeamInfo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AAtomTeamInfo, TeamId, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(AAtomTeamInfo, TeamColor, COND_InitialOnly);
	DOREPLIFETIME(AAtomTeamInfo, Score);
}
