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

void UGameModeUISubsystem::Destroy()
{
	ConditionalBeginDestroy();
}

class UWorld* UGameModeUISubsystem::GetWorld() const
{
	return UISystem ? UISystem->GetWorld() : nullptr;
}
