// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "ControlPointGameState.h"
#include "AtomControlPoint.h"

#define LOCTEXT_NAMESPACE "AtomGameState"

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

//FText AControlPointGameState::GetGameStatusText() const
//{
//	if (MatchState == MatchState::InProgress && 
//		ActiveControlPoint != nullptr && 
//		!ActiveControlPoint->IsActive())
//	{
//		return LOCTEXT("GameStatusWaitingObjective", "Waiting for Objective");
//	}
//
//	return Super::GetGameStatusText();
//}

#undef LOCTEXT_NAMESPACE
