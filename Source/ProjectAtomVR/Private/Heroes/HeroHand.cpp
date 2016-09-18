// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "HeroHand.h"


// Sets default values
AHeroHand::AHeroHand(const FObjectInitializer& ObjectInitializer/* = FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
 	// Set this component to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bCanBeDamaged = false;

	MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionController"));
	RootComponent = MotionController;

	DefaultMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("DefaultMesh"));
	DefaultMesh->SetupAttachment(RootComponent);
}

void AHeroHand::SetHandDominance(EHandedness Handedness, EDominance Dominance)
{
	MotionController->Hand = (Handedness == EHandedness::Left) ? EControllerHand::Left : EControllerHand::Right;

	if (Handedness == EHandedness::Left)
	{
		const FVector OriginalScale = DefaultMesh->GetComponentScale();
		DefaultMesh->SetRelativeScale3D(FVector{ OriginalScale.X, OriginalScale.Y, -OriginalScale.Z });
	}
}
