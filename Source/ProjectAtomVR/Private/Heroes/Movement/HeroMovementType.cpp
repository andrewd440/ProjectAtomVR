// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "HeroMovementType.h"

#include "HeroBase.h"

UHeroMovementType::UHeroMovementType(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{

}

void UHeroMovementType::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{

}

void UHeroMovementType::PostLoad()
{
	Super::PostLoad();

	Hero = Cast<AHeroBase>(GetOwner());
}
