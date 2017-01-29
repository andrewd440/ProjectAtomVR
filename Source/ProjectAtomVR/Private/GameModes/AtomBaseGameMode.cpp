// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomBaseGameMode.h"

#include "AtomPlayerController.h"
#include "AtomCharacter.h"
#include "VRHUD.h"
#include "AtomPlayerState.h"

AAtomBaseGameMode::AAtomBaseGameMode()
{
	GameStateClass = AAtomGameState::StaticClass();
	DefaultPawnClass = AAtomCharacter::StaticClass();
	PlayerControllerClass = AAtomPlayerController::StaticClass();
	PlayerStateClass = AAtomPlayerState::StaticClass();
	VRHUDClass = AVRHUD::StaticClass();

	bUseSeamlessTravel = false;
	bDelayCharacterLoadoutCreation = false;
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

TSubclassOf<class AVRHUD> AAtomBaseGameMode::GetVRHUDClass() const
{
	return VRHUDClass;
}

bool AAtomBaseGameMode::ShouldDelayCharacterLoadoutCreation() const
{
	return bDelayCharacterLoadoutCreation;
}

void AAtomBaseGameMode::DefaultTimer()
{
	FTimerManager& TimerManager = GetWorldTimerManager();
	TimerManager.SetTimer(TimerHandle_DefaultTimer, this, &AAtomBaseGameMode::DefaultTimer, GetWorldSettings()->GetEffectiveTimeDilation() / GetWorldSettings()->DemoPlayTimeDilation, true);

	CheckGameTime();
}

void AAtomBaseGameMode::CheckGameTime()
{

}

void AAtomBaseGameMode::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	FTimerManager& TimerManager = GetWorldTimerManager();
	TimerManager.SetTimer(TimerHandle_DefaultTimer, this, &AAtomBaseGameMode::DefaultTimer, GetWorldSettings()->GetEffectiveTimeDilation() / GetWorldSettings()->DemoPlayTimeDilation, true);
}

void AAtomBaseGameMode::SetMatchState(FName NewState)
{
	// Needed to reorder logic to set match state in gamestate before OnmatchStateSet to allow handlers to set match state
	// and be updated correctly in gamestate.
	if (MatchState == NewState)
	{
		return;
	}

	UE_LOG(LogGameMode, Display, TEXT("Match State Changed from %s to %s"), *MatchState.ToString(), *NewState.ToString());

	MatchState = NewState;

	AGameState* FullGameState = GetGameState<AGameState>();
	if (FullGameState)
	{
		FullGameState->SetMatchState(NewState);
	}	

	OnMatchStateSet();

	K2_OnSetMatchState(NewState);
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

void AAtomBaseGameMode::InitializeHUDForPlayer_Implementation(APlayerController* NewPlayer)
{
	// Tell client what HUD class to use
	if (auto AtomController = Cast<AAtomPlayerController>(NewPlayer))
	{
		AtomController->ClientSetVRHUD(VRHUDClass);
	}
	else
	{
		NewPlayer->ClientSetHUD(HUDClass);
	}	
}

void AAtomBaseGameMode::HandleMatchHasEnded()
{
	// Turn off all pawns. They will not be carried over to new map with seamless travel and new pawns will be created
	// for all controllers.
	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		(*It)->TurnOff();
	}

	Super::HandleMatchHasEnded();
}
