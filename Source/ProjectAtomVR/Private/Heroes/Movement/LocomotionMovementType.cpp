// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "LocomotionMovementType.h"

#include "HMDCameraComponent.h"

void ULocomotionMovementType::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	FName ForwardInput, RightInput;
	if (GetHero()->IsRightHanded())
	{
		ForwardInput = TEXT("MotionControllerThumbLeft_Y");
		RightInput = TEXT("MotionControllerThumbLeft_X");
	}
	else
	{
		ForwardInput = TEXT("MotionControllerThumbRight_Y");
		RightInput = TEXT("MotionControllerThumbRight_X");
	}

	InputComponent->BindAxis(ForwardInput, this, &ULocomotionMovementType::OnMoveForward);
	InputComponent->BindAxis(RightInput, this, &ULocomotionMovementType::OnMoveRight);
}

void ULocomotionMovementType::OnMoveForward(float Value)
{
	AHeroBase* const MyHero = GetHero();
	const FVector Direction = MyHero->GetCamera()->GetForwardVector();
	MyHero->AddMovementInput(Direction.GetSafeNormal2D(), Value);
}

void ULocomotionMovementType::OnMoveRight(float Value)
{
	AHeroBase* const MyHero = GetHero();
	const FVector Direction = MyHero->GetCamera()->GetRightVector();
	MyHero->AddMovementInput(Direction.GetSafeNormal2D(), Value);
}
