// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "ControlPointPlayerState.h"




void AControlPointPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AControlPointPlayerState, PointsCaptured);
	DOREPLIFETIME(AControlPointPlayerState, AttackKills);
	DOREPLIFETIME(AControlPointPlayerState, DefenseKills);
}
