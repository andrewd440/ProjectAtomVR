// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomPlayerController.h"
#include "AtomCharacter.h"
#include "UI/AtomUISystem.h"
#include "AtomLocalPlayer.h"

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
	const bool IsNewPawn = (AtomCharacter != aPawn);
	if (UISystem && AtomCharacter && IsNewPawn)
	{
		UISystem->DestroyHeroUI();
	}

	Super::SetPawn(aPawn);

	AtomCharacter = Cast<AAtomCharacter>(aPawn);

	if (AtomCharacter)
	{
		AtomCharacter->SetIsRightHanded(bIsRightHanded);

		if (UISystem && IsNewPawn)
		{
			UISystem->SpawnHeroUI();
		}
	}
}

void AAtomPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AAtomPlayerController, RequestedCharacter, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AAtomPlayerController, bIsRightHanded, COND_SkipOwner);
}

void AAtomPlayerController::SetPlayer(UPlayer* InPlayer)
{
	Super::SetPlayer(InPlayer);

	if (UAtomLocalPlayer* AtomPlayer = Cast<UAtomLocalPlayer>(InPlayer))
	{
		bIsRightHanded = AtomPlayer->GetIsRightHanded();
		ServerSetIsRightHanded(bIsRightHanded);
	}
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

void AAtomPlayerController::ServerSetIsRightHanded_Implementation(bool InbIsRightHanded)
{
	bIsRightHanded = InbIsRightHanded;
}

bool AAtomPlayerController::ServerSetIsRightHanded_Validate(bool InbIsRightHanded)
{
	return true;
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
	return AtomCharacter;
}

void AAtomPlayerController::SetRequestedCharacter(TSubclassOf<AAtomCharacter> CharacterClass)
{
	RequestedCharacter = CharacterClass;
}

TSubclassOf<AAtomCharacter> AAtomPlayerController::GetRequestedCharacter() const
{
	return RequestedCharacter;
}

bool AAtomPlayerController::IsRightHanded() const
{
	return bIsRightHanded;
}

