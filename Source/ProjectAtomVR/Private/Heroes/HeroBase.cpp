// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "HeroBase.h"

#include "HeroMovementType.h"
#include "HeroMovementComponent.h"
#include "NetMotionControllerComponent.h"
#include "NetCameraComponent.h"
#include "HMDCapsuleComponent.h"

// Sets default values
AHeroBase::AHeroBase(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UHeroMovementComponent>(ACharacter::CharacterMovementComponentName).DoNotCreateDefaultSubobject(ACharacter::MeshComponentName).SetDefaultSubobjectClass<UHMDCapsuleComponent>(ACharacter::CapsuleComponentName))
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = ETickingGroup::TG_PostPhysics;

	bReplicates = true;
	bReplicateMovement = true;

	VROrigin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));
	RootComponent = VROrigin;

	// Setup camera
	Camera = CreateDefaultSubobject<UNetCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(VROrigin);
	Camera->bLockToHmd = true;
	Camera->SetIsReplicated(true);

	// Setup head mesh
	HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
	HeadMesh->SetupAttachment(Camera);

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

	JumpMaxCount = 0;	

	UCapsuleComponent* const CapsuleComp = GetCapsuleComponent();
	CapsuleComp->SetupAttachment(Camera);
	GetMovementComponent()->SetUpdatedComponent(CapsuleComp);
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

void AHeroBase::PostNetReceiveLocationAndRotation()
{
	if (Role == ROLE_SimulatedProxy)
	{
		// Don't change transform if using relative position (it should be nearly the same anyway, or base may be slightly out of sync)
		if (!ReplicatedBasedMovement.HasRelativeLocation())
		{
			const UCapsuleComponent* const CapsuleComp = GetCapsuleComponent();
			const FVector NewCapsuleLocation = ReplicatedMovement.Location + CapsuleComp->RelativeLocation;

			const FVector OldLocation = GetActorLocation();
			const FQuat OldRotation = GetActorQuat();

			// Smooth using capsule params
			auto MovementComponent = GetHeroMovementComponent();
			MovementComponent->bNetworkSmoothingComplete = false;
			MovementComponent->SmoothCorrection(CapsuleComp->GetComponentLocation(), CapsuleComp->GetComponentQuat(), NewCapsuleLocation, ReplicatedMovement.Rotation.Quaternion());

			OnUpdateSimulatedPosition(OldLocation, OldRotation);
		}
	}
}

FVector AHeroBase::GetVelocity() const
{
	return GetCapsuleComponent()->GetComponentVelocity();
}

void AHeroBase::MovementTeleport(const FVector& DestLocation, const FRotator& DestRotation)
{
	// Get correct location and rotation
	const FVector CapsuleDestination = DestLocation + FVector{ 0, 0, GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() }; //- FVector{ Camera->RelativeLocation.X, Camera->RelativeLocation.Y, 0 };

	if (IsLocallyControlled())
	{
		// Set camera fade if local
		APlayerCameraManager* const PlayerCameraManager = static_cast<APlayerController*>(GetController())->PlayerCameraManager;

		// Fade camera
		PlayerCameraManager->StartCameraFade(0.f, 1.f, 0.1f, FLinearColor::Black, false, true);

		// Set delegate to finish the teleport
		FTimerDelegate FinishTeleportDelegate = FTimerDelegate::CreateUObject(this, &AHeroBase::FinishTeleport, CapsuleDestination, GetActorRotation());
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, FinishTeleportDelegate, 0.1f, false);
	}
	else
	{
		// Just teleport if server call and not locally controlled
		GetHeroMovementComponent()->TeleportMove(CapsuleDestination);
	}
}

void AHeroBase::FinishTeleport(FVector DestLocation, FRotator DestRotation)
{	
	GetHeroMovementComponent()->TeleportMove(DestLocation);

	check(IsLocallyControlled() && "Should only be called on locally controlled Heroes.");
	APlayerCameraManager* const PlayerCameraManager = static_cast<APlayerController*>(GetController())->PlayerCameraManager;
	PlayerCameraManager->StartCameraFade(1.f, 0.f, 0.2f, FLinearColor::Black);
}

FVector AHeroBase::GetPawnViewLocation() const
{
	return Camera->GetComponentLocation();
}

FRotator AHeroBase::GetViewRotation() const
{
	return Camera->GetComponentRotation();
}