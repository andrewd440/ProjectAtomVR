// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "HeroBase.h"

#include "HeroHand.h"
#include "HeroMovementType.h"
#include "HeroMovementComponent.h"

const FName AHeroBase::MovementTypeComponentName = TEXT("MovementType");

// Sets default values
AHeroBase::AHeroBase(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	VROrigin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));
	RootComponent = VROrigin;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(VROrigin);
	Camera->bLockToHmd = true;
	Camera->SetIsReplicated(true);
	
	MovementType = CreateDefaultSubobject<UHeroMovementType>(MovementTypeComponentName);
	MovementComponent = CreateDefaultSubobject<UHeroMovementComponent>(TEXT("MovementComponent"));
}

// Called when the game starts or when spawned
void AHeroBase::BeginPlay()
{
	Super::BeginPlay();

	if (GEngine->HMDDevice.IsValid())
	{
		GEngine->HMDDevice->SetTrackingOrigin(EHMDTrackingOrigin::Floor); // SteamVR and Rift origin is floor
	}
}

// Called every frame
void AHeroBase::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

// Called to bind functionality to input
void AHeroBase::SetupPlayerInputComponent(class UInputComponent* InInputComponent)
{
	Super::SetupPlayerInputComponent(InInputComponent);

	check(MovementType && "Movement type for hero is null. Please assign a valid movement type.");
	MovementType->SetupPlayerInputComponent(InInputComponent);
}

void AHeroBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (HasAuthority())
	{
		// Spawn each hero hand
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = this;

			if (DominateHandTemplate)
			{
				SpawnParams.Name = TEXT("DominateHand");
				DominateHand = GetWorld()->SpawnActor<AHeroHand>(DominateHandTemplate, SpawnParams);
				DominateHand->AttachToComponent(VROrigin, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

				DominateHand->SetHandDominance(bIsRightHanded ? AHeroHand::EHandedness::Right : AHeroHand::EHandedness::Left, AHeroHand::EDominance::Dominate);
			}

			if (NonDominateHandTemplate)
			{
				SpawnParams.Name = TEXT("NonDominateHand");
				NonDominateHand = GetWorld()->SpawnActor<AHeroHand>(NonDominateHandTemplate, SpawnParams);
				NonDominateHand->AttachToComponent(VROrigin, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

				NonDominateHand->SetHandDominance(bIsRightHanded ? AHeroHand::EHandedness::Left : AHeroHand::EHandedness::Right, AHeroHand::EDominance::NonDominate);
			}
		}
	}
}

UPawnMovementComponent* AHeroBase::GetMovementComponent() const
{
	return MovementComponent;
}
