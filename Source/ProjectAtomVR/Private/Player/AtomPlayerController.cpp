// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomPlayerController.h"
#include "AtomCharacter.h"
#include "UI/AtomUISystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogAtomPlayerController, Log, All);

AAtomPlayerController::AAtomPlayerController()
{

}

void AAtomPlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	CreateUISystem();
}

void AAtomPlayerController::SetPawn(APawn* aPawn)
{
	AAtomCharacter* NewHero = Cast<AAtomCharacter>(aPawn);
	if (UISystem && Hero && Hero != NewHero)
	{
		UISystem->DestroyHeroUI();
	}

	Super::SetPawn(aPawn);

	if (UISystem && NewHero && Hero != NewHero)
	{
		Hero = NewHero;
		UISystem->SpawnHeroUI();
	}
	else
	{
		Hero = NewHero;
	}
}

void AAtomPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AAtomPlayerController, RequestedCharacter, COND_OwnerOnly);
}

void AAtomPlayerController::CreateUISystem()
{
	if (IsLocalController())
	{
		UE_LOG(LogAtomPlayerController, Verbose, TEXT("Spawned UISystem"));
		UISystem = NewObject<UAtomUISystem>(this, TEXT("UISystem"), RF_Transient);
		UISystem->SetOwner(this);
	}
}

void AAtomPlayerController::execRequestCharacterChange(FString Name)
{
	UClass* Class = FindObjectFast<UClass>(nullptr, *Name, false, true);

	if (Class && Class->IsChildOf(AAtomCharacter::StaticClass()))
	{
		ServerRequestCharacterChange(Class);
	}
}

void AAtomPlayerController::ServerRequestCharacterChange_Implementation(TSubclassOf<AAtomCharacter> CharacterClass)
{
	if (AAtomGameMode* AtomGameMode = GetWorld()->GetAuthGameMode<AAtomGameMode>())
	{
		AtomGameMode->RequestCharacterChange(this, CharacterClass);
	}
}

bool AAtomPlayerController::ServerRequestCharacterChange_Validate(TSubclassOf<AAtomCharacter> CharacterClass)
{
	return true;
}

AAtomCharacter* AAtomPlayerController::GetHero() const
{
	return Hero;
}

void AAtomPlayerController::SetRequestedCharacter(TSubclassOf<AAtomCharacter> CharacterClass)
{
	RequestedCharacter = CharacterClass;
}

TSubclassOf<AAtomCharacter> AAtomPlayerController::GetRequestedCharacter() const
{
	return RequestedCharacter;
}

