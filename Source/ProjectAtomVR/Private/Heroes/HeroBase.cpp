// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "HeroBase.h"

#include "HeroHand.h"
#include "HeroMovementType.h"
#include "HeroMovementComponent.h"

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

	// Setup all movement types component input bindings
	TInlineComponentArray<UHeroMovementType*> MovementTypeComponents;
	GetComponents(MovementTypeComponents);

	for (auto MovementType : MovementTypeComponents)
	{
		MovementType->SetupPlayerInputComponent(InInputComponent);
	}
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

bool AHeroBase::TeleportTo(const FVector& DestLocation, const FRotator& DestRotation, bool bIsATest /*= false*/, bool bNoCheck /*= false*/)
{
	// Get correct location and rotation
	const FVector RootDestination = DestLocation - FVector{ Camera->RelativeLocation.X, Camera->RelativeLocation.Y, 0 };

	if (IsLocallyControlled())
	{
		// Set camera fade if local
		APlayerCameraManager* const PlayerCameraManager = static_cast<APlayerController*>(GetController())->PlayerCameraManager;

		// Fade camera
		PlayerCameraManager->StartCameraFade(0.f, 1.f, 0.1f, FLinearColor::Black, false, true);

		// Set delegate to finish the teleport
		FTimerDelegate FinishTeleportDelegate = FTimerDelegate::CreateUObject(this, &AHeroBase::FinishTeleport, RootDestination, GetActorRotation());
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, FinishTeleportDelegate, 0.1f, false);
		return true;
	}
	else
	{
		// Just teleport if server call
		return Super::TeleportTo(RootDestination, GetActorRotation(), bIsATest, bNoCheck);
	}
}

void AHeroBase::FinishTeleport(FVector DestLocation, FRotator DestRotation)
{	
	Super::TeleportTo(DestLocation, DestRotation, false, false); // if true, call server teleport

	check(IsLocallyControlled() && "Should only be called on locally controlled Heroes.");
	APlayerCameraManager* const PlayerCameraManager = static_cast<APlayerController*>(GetController())->PlayerCameraManager;
	PlayerCameraManager->StartCameraFade(1.f, 0.f, 0.2f, FLinearColor::Black);
}
