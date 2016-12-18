// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "MainMenuUISubsystem.h"

namespace
{
	static const FName MainMenuLocatorTag{ TEXT("MainMenu") };
}


void UMainMenuUISubsystem::InitializeSystem(AAtomUISystem* Owner, AAtomGameMode* GameModeCDO)
{
	Super::InitializeSystem(Owner, GameModeCDO);

	if (MainMenuWidget != nullptr)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetUISystem();
		SpawnParams.ObjectFlags |= RF_Transient;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		USceneComponent* MainMenuLocator = GetUISystem()->FindFirstUILocator(MainMenuLocatorTag);

		MainMenuUI = GetWorld()->SpawnActor<AAtomFloatingUI>(AAtomFloatingUI::StaticClass(), MainMenuLocator ? MainMenuLocator->GetComponentTransform() : FTransform::Identity, SpawnParams);
		
		MainMenuUI->SetWidget(GetUISystem(), MainMenuWidget);
	}
}

void UMainMenuUISubsystem::Destroy()
{
	if (MainMenuUI != nullptr)
	{
		MainMenuUI->Destroy();
		MainMenuUI = nullptr;
	}

	Super::Destroy();
}
