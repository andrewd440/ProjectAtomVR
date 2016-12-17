// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "LocomotionMovementType.h"

#include "HMDCameraComponent.h"

ULocomotionMovementType::ULocomotionMovementType()
	: bIsGripPressed(false)
{

}

void ULocomotionMovementType::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	FName ForwardInput, RightInput, GripInput;
	if (GetCharacter()->IsRightHanded())
	{
		ForwardInput = TEXT("MotionControllerThumbLeft_Y");
		RightInput = TEXT("MotionControllerThumbLeft_X");
		GripInput = TEXT("GripLeft");
	}
	else
	{
		ForwardInput = TEXT("MotionControllerThumbRight_Y");
		RightInput = TEXT("MotionControllerThumbRight_X");
		GripInput = TEXT("GripRight");
	}

	InputComponent->BindAxis(ForwardInput, this, &ULocomotionMovementType::OnMoveForward);
	InputComponent->BindAxis(RightInput, this, &ULocomotionMovementType::OnMoveRight);
	InputComponent->BindAction(GripInput, IE_Pressed, this, &ULocomotionMovementType::OnGripPressed);
	InputComponent->BindAction(GripInput, IE_Released, this, &ULocomotionMovementType::OnGripReleased);
}

void ULocomotionMovementType::OnMoveForward(float Value)
{
	AAtomCharacter* const MyCharacter = GetCharacter();

	if (!bIsGripPressed)
	{
		const FVector Direction = MyCharacter->GetBodyMesh()->GetForwardVector();
		MyCharacter->AddMovementInput(Direction.GetSafeNormal2D(), Value);
	}
}

void ULocomotionMovementType::OnMoveRight(float Value)
{
	AAtomCharacter* const MyCharacter = GetCharacter();

	if (!bIsGripPressed)
	{
		const FVector Direction = MyCharacter->GetBodyMesh()->GetRightVector();
		MyCharacter->AddMovementInput(Direction.GetSafeNormal2D(), Value);
	}
}

void ULocomotionMovementType::OnGripPressed()
{
	bIsGripPressed = true;
}

void ULocomotionMovementType::OnGripReleased()
{
	bIsGripPressed = false;
}
