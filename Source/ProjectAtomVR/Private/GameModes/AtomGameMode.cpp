// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomGameMode.h"
#include "AtomPlayerController.h"
#include "AtomCharacter.h"
#include "AtomGameInstance.h"

AAtomGameMode::AAtomGameMode()
{

}

void AAtomGameMode::ScoreKill_Implementation(APlayerController* Killer, APlayerController* Victim)
{

}

bool AAtomGameMode::IsCharacterChangeAllowed_Implementation(AAtomPlayerController*) const
{
	return false; // Default to false
}

bool AAtomGameMode::ReadyToEndMatch_Implementation()
{
	if (Super::ReadyToEndMatch_Implementation())
	{
		return true;
	}
	else if (TimeLimit > 0)
	{
		if (AAtomGameState* const AtomGameState = GetGameState<AAtomGameState>())
		{
			return AtomGameState->ElapsedTime >= TimeLimit;
		}		
	}

	return false;
}

void AAtomGameMode::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();

	// Travel back to lobby
	check(Cast<UAtomGameInstance>(GetGameInstance()));

	UAtomGameInstance* const GameInstance = static_cast<UAtomGameInstance*>(GetGameInstance());

	const FString URLString = FString::Printf(TEXT("/Game/Maps/%s?listen?game=%s"), *GameInstance->GetLobbyMap().ToString(), *GameInstance->GetLobbyGameMode().ToString());
	GetWorld()->ServerTravel(URLString);
}

