// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "GameModeUISubsystem.h"




UGameModeUISubsystem::UGameModeUISubsystem()
	: bShowPlayerNames(false)
{

}

void UGameModeUISubsystem::InitializeSystem(AAtomUISystem* Owner, AAtomGameMode* GameModeCDO)
{
	UISystem = Owner;

	GameState = GetWorld()->GetGameState<AAtomGameState>();
}

class UWorld* UGameModeUISubsystem::GetWorld() const
{
	return UISystem->GetWorld();
}
