// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomCharacterMovementType.h"

#include "AtomCharacter.h"

UAtomCharacterMovementType::UAtomCharacterMovementType(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{

}

void UAtomCharacterMovementType::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{

}

void UAtomCharacterMovementType::PostLoad()
{
	Super::PostLoad();

	Character = Cast<AAtomCharacter>(GetOwner());
}
