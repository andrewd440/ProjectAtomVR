// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "ControlPointGameState.h"




AControlPointGameState::AControlPointGameState()
{

}

void AControlPointGameState::SetActiveControlPoint(AAtomControlPoint* ControlPoint)
{
	ActiveControlPoint = ControlPoint;
}

AAtomControlPoint* AControlPointGameState::GetActiveControlPoint() const
{
	return ActiveControlPoint;
}

void AControlPointGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AControlPointGameState, ActiveControlPoint);
}
