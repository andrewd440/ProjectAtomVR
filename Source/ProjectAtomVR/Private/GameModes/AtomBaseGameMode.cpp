// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomBaseGameMode.h"

#include "AtomPlayerController.h"
#include "AtomCharacter.h"

AAtomBaseGameMode::AAtomBaseGameMode()
{
	GameStateClass = AAtomGameState::StaticClass();
	DefaultPawnClass = AAtomCharacter::StaticClass();
	PlayerControllerClass = AAtomPlayerController::StaticClass();
}

void AAtomBaseGameMode::RequestCharacterChange(AAtomPlayerController* Controller, TSubclassOf<class AAtomCharacter> Character)
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

TSubclassOf<UGameModeUISubsystem> AAtomBaseGameMode::GetUIClass() const
{
	return UIClass;
}

bool AAtomBaseGameMode::IsCharacterChangeAllowed_Implementation(AAtomPlayerController*) const
{
	return true; // Default to true
}

UClass* AAtomBaseGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
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
