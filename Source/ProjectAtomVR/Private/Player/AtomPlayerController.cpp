// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomPlayerController.h"
#include "HeroBase.h"
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
	AHeroBase* NewHero = Cast<AHeroBase>(aPawn);
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

void AAtomPlayerController::CreateUISystem()
{
	if (IsLocalController())
	{
		UE_LOG(LogAtomPlayerController, Verbose, TEXT("Spawned UISystem"));
		UISystem = NewObject<UAtomUISystem>(this, TEXT("UISystem"), RF_Transient);
		UISystem->SetOwner(this);
	}
}

AHeroBase* AAtomPlayerController::GetHero() const
{
	return Hero;
}

