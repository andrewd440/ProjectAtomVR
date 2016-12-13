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
	return Super::ReadyToEndMatch() || GetGameState<AGameState>()->ElapsedTime > TimeLimit;
}
