// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "HeroBase.h"

#include "HeroMovementType.h"
#include "HeroMovementComponent.h"
#include "MotionComponents/NetMotionControllerComponent.h"
#include "MotionComponents/NetCameraComponent.h"

// Sets default values
AHeroBase::AHeroBase(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	VROrigin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));
	RootComponent = VROrigin;

	// Setup camera
	Camera = CreateDefaultSubobject<UNetCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(VROrigin);
	Camera->bLockToHmd = true;
	Camera->SetIsReplicated(true);
	
	// Setup movement
	MovementComponent = CreateDefaultSubobject<UHeroMovementComponent>(TEXT("MovementComponent"));

	// Setup left hand
	LeftHandController = CreateDefaultSubobject<UNetMotionControllerComponent>(TEXT("DominateHandController"));
	LeftHandController->Hand = EControllerHand::Left;
	LeftHandController->SetupAttachment(VROrigin);
	LeftHandController->SetIsReplicated(true);

	LeftHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("DominateHandMesh"));
	LeftHandMesh->SetupAttachment(LeftHandController);

	// Setup right hand
	RightHandController = CreateDefaultSubobject<UNetMotionControllerComponent>(TEXT("NonDominateHandController"));
	RightHandController->Hand = EControllerHand::Right;
	RightHandController->SetupAttachment(VROrigin);
	RightHandController->SetIsReplicated(true);
	
	RightHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("NonDominateHandMesh"));
	RightHandMesh->SetupAttachment(RightHandController);
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
}

UPawnMovementComponent* AHeroBase::GetMovementComponent() const
{
	return MovementComponent;
}

void AHeroBase::MovementTeleport(const FVector& DestLocation, const FRotator& DestRotation)
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
	}
	else
	{
		// Just teleport if server call and not locally controlled
		TeleportTo(RootDestination, GetActorRotation());
	}
}

void AHeroBase::FinishTeleport(FVector DestLocation, FRotator DestRotation)
{	
	if (TeleportTo(DestLocation, DestRotation) && !HasAuthority())
	{
		// Only send to server if teleport was valid
		ServerMovementTeleport(DestLocation, DestRotation);
	}

	check(IsLocallyControlled() && "Should only be called on locally controlled Heroes.");
	APlayerCameraManager* const PlayerCameraManager = static_cast<APlayerController*>(GetController())->PlayerCameraManager;
	PlayerCameraManager->StartCameraFade(1.f, 0.f, 0.2f, FLinearColor::Black);
}

void AHeroBase::ServerMovementTeleport_Implementation(const FVector& DestLocation, const FRotator& DestRotation)
{
	TeleportTo(DestLocation, DestRotation);
}

bool AHeroBase::ServerMovementTeleport_Validate(const FVector& DestLocation, const FRotator& DestRotation)
{
	// #AtomTodo Maybe check teleport range and DestLocation validity
	return true;
}