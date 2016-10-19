// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "HeroBase.h"

#include "HeroMovementType.h"
#include "HeroMovementComponent.h"
#include "NetMotionControllerComponent.h"
#include "HMDCapsuleComponent.h"
#include "HMDCameraComponent.h"
#include "HeroLoadout.h"
#include "Engine/ActorChannel.h"
#include "HeroEquippable.h"
#include "Animation/AnimSequence.h"

DEFINE_LOG_CATEGORY_STATIC(LogHero, Log, All);

namespace
{
	static constexpr float HeadOrientationFactor = 0.5f; // Influence that the head orientation has on the body mesh
	static constexpr float HandsOrientationFactor = 1.f - HeadOrientationFactor; // Influence that the direction of hands has on the body mesh.
}

// Sets default values
AHeroBase::AHeroBase(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UHeroMovementComponent>(ACharacter::CharacterMovementComponentName).DoNotCreateDefaultSubobject(ACharacter::MeshComponentName).SetDefaultSubobjectClass<UHMDCapsuleComponent>(ACharacter::CapsuleComponentName))
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = ETickingGroup::TG_PostPhysics;

	bReplicates = true;
	bReplicateMovement = true;

	// Setup camera
	Camera = CreateDefaultSubobject<UHMDCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(RootComponent);
	Camera->bLockToHmd = true;
	Camera->SetIsReplicated(true);

	// Setup head mesh
	HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
	HeadMesh->SetupAttachment(Camera);
	HeadMesh->bOwnerNoSee = true;

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->bAbsoluteLocation = true;
	BodyMesh->bAbsoluteRotation = true;

	// Setup left hand
	LeftHandController = CreateDefaultSubobject<UNetMotionControllerComponent>(TEXT("LeftHandController"));
	LeftHandController->Hand = EControllerHand::Left;
	LeftHandController->SetupAttachment(RootComponent);
	LeftHandController->SetIsReplicated(true);

	LeftHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LeftHandMesh"));
	LeftHandMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	LeftHandMesh->SetAnimation(AnimDefaultHand);
	LeftHandMesh->SetupAttachment(LeftHandController);
	LeftHandMesh->bGenerateOverlapEvents = true;
	LeftHandMesh->SetCollisionProfileName(AtomCollisionProfiles::HeroHand);

	// Setup right hand
	RightHandController = CreateDefaultSubobject<UNetMotionControllerComponent>(TEXT("RightHandController"));
	RightHandController->Hand = EControllerHand::Right;
	RightHandController->SetupAttachment(RootComponent);
	RightHandController->SetIsReplicated(true);
	
	RightHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RightHandMesh"));
	RightHandMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	RightHandMesh->SetAnimation(AnimDefaultHand);
	RightHandMesh->SetupAttachment(RightHandController);
	RightHandMesh->bGenerateOverlapEvents = true;
	RightHandMesh->SetCollisionProfileName(AtomCollisionProfiles::HeroHand);

	// Setup loadout
	Loadout = CreateDefaultSubobject<UHeroLoadout>(TEXT("Loadout"));

	JumpMaxCount = 0;	
}

void AHeroBase::BeginPlay()
{
	Super::BeginPlay();

	if (GEngine->HMDDevice.IsValid())
	{
		GEngine->HMDDevice->SetTrackingOrigin(EHMDTrackingOrigin::Floor); // SteamVR and Rift origin is floor
	}		

	Loadout->InitializeLoadout(this);
}

void AHeroBase::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	UpdateBodyMeshLocation();
}

void AHeroBase::UpdateBodyMeshLocation()
{
	const FVector NeckBaseLocation = Camera->GetWorldNeckBaseLocation();
	const FVector CameraForward2D = Camera->GetForwardVector().GetSafeNormal2D();

	// Get the averaged controller direction from the two hand controllers
	const FVector RightControllerDirection2D = (RightHandController->GetComponentLocation() - NeckBaseLocation).GetSafeNormal2D();
	const FVector LeftControllerDirection2D = (LeftHandController->GetComponentLocation() - NeckBaseLocation).GetSafeNormal2D();

	FVector ControllerForward2D = (RightControllerDirection2D + LeftControllerDirection2D) / 2.f;

	// If the controller forward is not on the front side of the camera forward, reflect it so that it is.
	const float CameraDotController = FVector::DotProduct(CameraForward2D, ControllerForward2D);
	if (CameraDotController < 0.f)
	{
		ControllerForward2D += 2.f * CameraForward2D;
	}

	const FVector BodyForward2D = CameraForward2D * HeadOrientationFactor + ControllerForward2D * HandsOrientationFactor;

	const FVector BodyLocation = NeckBaseLocation - NeckBaseSocketLocation;
	BodyMesh->SetWorldLocationAndRotation(BodyLocation, BodyForward2D.ToOrientationQuat());
}

void AHeroBase::SetupPlayerInputComponent(class UInputComponent* InInputComponent)
{
	Super::SetupPlayerInputComponent(InInputComponent);

	// Bind inputs for hero
	InInputComponent->BindAction(TEXT("EquipLeft"), EInputEvent::IE_Pressed, this, &AHeroBase::OnEquipPressed<EHand::Left>);
	InInputComponent->BindAction(TEXT("EquipRight"), EInputEvent::IE_Pressed, this, &AHeroBase::OnEquipPressed<EHand::Right>);

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

	NeckBaseSocketLocation = BodyMesh->GetSocketTransform(NeckBaseSocket, RTS_Component).GetTranslation();
}

void AHeroBase::PostNetReceiveLocationAndRotation()
{
	if (Role == ROLE_SimulatedProxy)
	{
		// Don't change transform if using relative position (it should be nearly the same anyway, or base may be slightly out of sync)
		if (!ReplicatedBasedMovement.HasRelativeLocation())
		{
			const FVector OldLocation = GetActorLocation();
			const FQuat OldRotation = GetActorQuat();

			SetActorLocationAndRotation(ReplicatedMovement.Location, ReplicatedMovement.Rotation);

			OnUpdateSimulatedPosition(OldLocation, OldRotation);
		}
	}
}

FVector AHeroBase::GetVelocity() const
{
	return GetCapsuleComponent()->GetComponentVelocity();
}

bool AHeroBase::ReplicateSubobjects(UActorChannel *Channel, FOutBunch *Bunch, FReplicationFlags *RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	bWroteSomething |= Channel->ReplicateSubobject(Loadout, *Bunch, *RepFlags);

	return bWroteSomething;
}

void AHeroBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(AHeroBase, Loadout);
	DOREPLIFETIME_CONDITION(AHeroBase, RightHandEquippable, COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(AHeroBase, LeftHandEquippable, COND_SimulatedOnly);
}

void AHeroBase::MovementTeleport(const FVector& DestLocation, const FRotator& DestRotation)
{
	// Get correct location and rotation
	FVector DestinationOffset = GetActorLocation() - Camera->GetWorldHeadLocation();
	DestinationOffset.Z = 0.f;
	const FVector CapsuleDestination = DestLocation + DestinationOffset;

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

UHMDCapsuleComponent* AHeroBase::GetHMDCapsuleComponent() const
{
	return static_cast<UHMDCapsuleComponent*>(GetCapsuleComponent());
}

void AHeroBase::Equip(AHeroEquippable* Item, const EHand Hand)
{ 
	if (Role == ENetRole::ROLE_AutonomousProxy)
	{
		ServerEquip(Item, Hand);
	}

	AHeroEquippable*& EquippablePtr = (Hand == EHand::Left) ? LeftHandEquippable : RightHandEquippable;

	if (EquippablePtr != nullptr)
	{
		EquippablePtr->Unequip();
	}

	EquippablePtr = Item;
	EquippablePtr->Equip(Hand);
}

void AHeroBase::ServerEquip_Implementation(class AHeroEquippable* Item, const EHand Hand)
{
	Equip(Item, Hand);
}

bool AHeroBase::ServerEquip_Validate(class AHeroEquippable* Item, const EHand Hand)
{
	return true;
}

void AHeroBase::Unequip(AHeroEquippable* Item)
{
	if (Role == ENetRole::ROLE_AutonomousProxy)
	{
		ServerUnequip(Item);
	}

	check(Item == LeftHandEquippable || Item == RightHandEquippable);
	EHand Hand = (Item == LeftHandEquippable) ? EHand::Left : EHand::Right;

	if (Hand == EHand::Left)
	{		
		LeftHandEquippable->Unequip();
		LeftHandEquippable = nullptr;

		LeftHandMesh->PlayAnimation(AnimDefaultHand, true);
	}	
	else
	{
		RightHandEquippable->Unequip();
		RightHandEquippable = nullptr;

		RightHandMesh->PlayAnimation(AnimDefaultHand, true);
	}
}

void AHeroBase::ServerUnequip_Implementation(class AHeroEquippable* Item)
{
	Unequip(Item);
}

bool AHeroBase::ServerUnequip_Validate(class AHeroEquippable* Item)
{
	return true;
}

template <EHand Hand>
void AHeroBase::OnEquipPressed()
{
	if (Hand == EHand::Left)
	{
		if (LeftHandEquippable == nullptr)
		{
			Loadout->RequestEquip(LeftHandMesh, Hand);
		}
		else
		{
			Loadout->RequestUnequip(LeftHandMesh, LeftHandEquippable);
		}
	}
	else
	{
		if (RightHandEquippable == nullptr)
		{
			Loadout->RequestEquip(RightHandMesh, Hand);
		}
		else
		{
			Loadout->RequestUnequip(RightHandMesh, RightHandEquippable);
		}
	}	
}

void AHeroBase::FinishTeleport(FVector DestLocation, FRotator DestRotation)
{	
	GetHeroMovementComponent()->TeleportMove(DestLocation);

	check(IsLocallyControlled() && "Should only be called on locally controlled Heroes.");
	APlayerCameraManager* const PlayerCameraManager = static_cast<APlayerController*>(GetController())->PlayerCameraManager;
	PlayerCameraManager->StartCameraFade(1.f, 0.f, 0.2f, FLinearColor::Black);
}

void AHeroBase::OnRep_LeftHandEquippable()
{
	
}

void AHeroBase::OnRep_RightHandEquippable()
{

}

FVector AHeroBase::GetPawnViewLocation() const
{
	return Camera->GetComponentLocation();
}

FRotator AHeroBase::GetViewRotation() const
{
	return Camera->GetComponentRotation();
}