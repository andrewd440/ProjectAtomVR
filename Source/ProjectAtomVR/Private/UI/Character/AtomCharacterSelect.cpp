// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomCharacterSelect.h"
#include "WidgetComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Character/CharacterSelectWidget.h"


// Sets default values
AAtomCharacterSelect::AAtomCharacterSelect()
{
	bReplicates = false;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->bCastDynamicShadow = false;
	RootComponent = Mesh;

	SelectionWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("SelectionWidgetComponent"));
	SelectionWidgetComponent->SetupAttachment(Mesh);
	SelectionWidgetComponent->SetCastShadow(false);
}

TSubclassOf<AAtomCharacter> AAtomCharacterSelect::GetCharacterClass() const
{
	return CharacterClass;
}

void AAtomCharacterSelect::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if(UCharacterSelectWidget* SelectionWidget = Cast<UCharacterSelectWidget>(SelectionWidgetComponent->GetUserWidgetObject()))
	{		
		SelectionWidget->SetCharacterClass(CharacterClass);
	}
}
