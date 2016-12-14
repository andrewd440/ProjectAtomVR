// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomGameMode.h"

AAtomGameMode::AAtomGameMode()
{
	GameStateClass = AAtomGameState::StaticClass();
	DefaultPawnClass = AHeroBase::StaticClass();
	PlayerControllerClass = AAtomPlayerController::StaticClass();
}

void AAtomGameMode::ScoreKill_Implementation(APlayerController* Killer, APlayerController* Victim)
{

}

bool AAtomGameMode::ReadyToEndMatch_Implementation()
{
	ensure(GetGameState<AGameState>());

	if (Super::ReadyToEndMatch_Implementation())
	{
		return true;
	}
	else if (AAtomGameState* const AtomGameState = GetGameState<AAtomGameState>())
	{
		return AtomGameState->ElapsedTime >= TimeLimit;
	}
}
