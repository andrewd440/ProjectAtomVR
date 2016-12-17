// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomGameMode.h"
#include "AtomPlayerController.h"
#include "AtomCharacter.h"

AAtomGameMode::AAtomGameMode()
{
	GameStateClass = AAtomGameState::StaticClass();
	DefaultPawnClass = AAtomCharacter::StaticClass();
	PlayerControllerClass = AAtomPlayerController::StaticClass();
}

void AAtomGameMode::RequestCharacterChange(AAtomPlayerController* Controller, TSubclassOf<class AAtomCharacter> Character)
{
	Controller->SetRequestedCharacter(Character);

	if (IsCharacterChangeAllowed(Controller))
	{
		// Unposses and destroy existing pawn
		APawn* Pawn = Controller->GetPawn();
		Controller->UnPossess();

		if (Pawn)
		{
			Pawn->Destroy(true);
		}

		RestartPlayer(Controller);
	}
}

TSubclassOf<UGameModeUISubsystem> AAtomGameMode::GetUIClass() const
{
	return UIClass;
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
	ensure(GetGameState<AGameState>());

	if (Super::ReadyToEndMatch_Implementation())
	{
		return true;
	}
	else if (AAtomGameState* const AtomGameState = GetGameState<AAtomGameState>())
	{
		return AtomGameState->ElapsedTime >= TimeLimit;
	}

	return false;
}

UClass* AAtomGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (AAtomPlayerController* AtomController = Cast<AAtomPlayerController>(InController))
	{
		if (AtomController->GetRequestedCharacter() != nullptr)
		{
			return AtomController->GetRequestedCharacter();
		}
	}

	return DefaultPawnClass;
}
