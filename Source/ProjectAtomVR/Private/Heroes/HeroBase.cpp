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
	static constexpr float HeadOrientationFactor = 0.35f; // Influence that the head orientation has on the body mesh
	static constexpr float HandsOrientationFactor = 1.f - HeadOrientationFactor; // Influence that the direction of hands has on the body mesh.
}

// Sets default values
AHeroBase::AHeroBase(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UHeroMovementComponent>(ACharacter::CharacterMovementComponentName).SetDefaultSubobjectClass<UHMDCapsuleComponent>(ACharacter::CapsuleComponentName))
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = ETickingGroup::TG_PostPhysics;

	bReplicates = true;
	bReplicateMovement = true;

	GetMesh()->SetOwnerNoSee(true);

	// Setup camera
	Camera = CreateDefaultSubobject<UHMDCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(RootComponent);
	Camera->bLockToHmd = true;
	Camera->SetIsReplicated(true);

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetOnlyOwnerSee(true);
	BodyMesh->bAbsoluteLocation = true;
	BodyMesh->bAbsoluteRotation = true;

	// Setup left hand
	LeftHandController = CreateDefaultSubobject<UNetMotionControllerComponent>(TEXT("LeftHandController"));
	LeftHandController->Hand = EControllerHand::Left;
	LeftHandController->SetupAttachment(RootComponent);
	LeftHandController->SetIsReplicated(true);

	LeftHandTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("LeftHandTrigger"));		
	LeftHandTrigger->SetupAttachment(LeftHandController);
	LeftHandTrigger->SetRelativeLocation(FVector{ -10.8, 1, -6.9 });
	LeftHandTrigger->SetIsReplicated(false);
	LeftHandTrigger->SetSphereRadius(4.f);
	LeftHandTrigger->bGenerateOverlapEvents = true;
	LeftHandTrigger->SetCollisionProfileName(AtomCollisionProfiles::HeroHand);

	LeftHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LeftHandMesh"));
	LeftHandMesh->SetOnlyOwnerSee(true);
	LeftHandMesh->SetupAttachment(LeftHandController);
	LeftHandMesh->SetRelativeLocationAndRotation(FVector{ -20.2, -1.7, 3.3 }, FRotator{ -40, 0, -90 });
	LeftHandMesh->SetIsReplicated(false);
	LeftHandMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	LeftHandMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

	// Setup right hand
	RightHandController = CreateDefaultSubobject<UNetMotionControllerComponent>(TEXT("RightHandController"));
	RightHandController->Hand = EControllerHand::Right;
	RightHandController->SetupAttachment(RootComponent);
	RightHandController->SetIsReplicated(true);
	
	RightHandTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("RightHandTrigger"));
	RightHandTrigger->SetupAttachment(RightHandController);
	RightHandTrigger->SetRelativeLocation(FVector{ -10.8, 1, -6.9 });
	RightHandTrigger->SetIsReplicated(false);
	RightHandTrigger->SetSphereRadius(4.f);
	RightHandTrigger->bGenerateOverlapEvents = true;
	RightHandTrigger->SetCollisionProfileName(AtomCollisionProfiles::HeroHand);

	RightHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RightHandMesh"));
	RightHandMesh->SetOnlyOwnerSee(true);
	RightHandMesh->SetupAttachment(RightHandController);
	RightHandMesh->SetRelativeLocationAndRotation(FVector{ -20.2, 1.7, 3.3 }, FRotator{ -40, 0, 90 });
	RightHandMesh->SetIsReplicated(false);
	RightHandMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	RightHandMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

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

	Loadout->SpawnLoadout();
}

void AHeroBase::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	UpdateMeshLocation(DeltaTime);
}

void AHeroBase::UpdateMeshLocation(float DeltaTime)
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

	FVector BodyLocation = NeckBaseLocation - NeckBaseSocketLocation;
	FQuat BodyRotation = BodyForward2D.ToOrientationQuat();

	// Calc movement velocity
	RoomScaleVelocity = (BodyLocation - BodyMesh->GetComponentLocation()) / DeltaTime;

	BodyMesh->SetWorldLocationAndRotation(BodyLocation, BodyRotation);

	// Update full body location using only xy for location and yaw rotation
	USkeletalMeshComponent* FullBodyMesh = GetMesh();
	BodyLocation.Z = FullBodyMesh->GetComponentLocation().Z;
	FullBodyMesh->SetWorldLocationAndRotation(BodyLocation, FRotator{0, BodyRotation.Rotator().Yaw, 0});
}

void AHeroBase::SetupPlayerInputComponent(class UInputComponent* InInputComponent)
{
	Super::SetupPlayerInputComponent(InInputComponent);

	// Bind inputs for hero
	InInputComponent->BindAction(TEXT("GripLeft"), EInputEvent::IE_Pressed, this, &AHeroBase::OnEquipPressed<EHand::Left>);
	InInputComponent->BindAction(TEXT("GripRight"), EInputEvent::IE_Pressed, this, &AHeroBase::OnEquipPressed<EHand::Right>);

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

	DefaultLeftHandTransform.Location = LeftHandMesh->RelativeLocation;
	DefaultLeftHandTransform.Rotation = LeftHandMesh->RelativeRotation;

	DefaultRightHandTransform.Location = RightHandMesh->RelativeLocation;
	DefaultRightHandTransform.Rotation = RightHandMesh->RelativeRotation;

	Loadout->InitializeLoadout(this);
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

USceneComponent* AHeroBase::GetHandMeshTarget(const EHand Hand) const
{
	return (Hand == EHand::Left) ? LeftHandMesh : RightHandMesh;
}

void AHeroBase::Equip(AHeroEquippable* Item, const EHand Hand)
{ 
	// Unequip first if hand is in use
	AHeroEquippable*& EquippablePtr = (Hand == EHand::Left) ? LeftHandEquippable : RightHandEquippable;

	if (EquippablePtr != nullptr)
	{
		EquippablePtr->Unequip();
	}

	Item->Equip(Hand);
}

void AHeroBase::OnEquipped(AHeroEquippable* Item, const EHand Hand)
{
	if (Hand == EHand::Left)
	{
		LeftHandEquippable = Item;
	}
	else
	{
		RightHandEquippable = Item;
	}
}

void AHeroBase::Unequip(AHeroEquippable* Item, const EHand Hand)
{
	ensure(Item == LeftHandEquippable || Item == RightHandEquippable);

	if (Hand == EHand::Left)
	{		
		LeftHandEquippable->Unequip();
	}	
	else
	{
		RightHandEquippable->Unequip();
	}
}

void AHeroBase::OnUnequipped(AHeroEquippable* Item, const EHand Hand)
{
	ensure(Item == LeftHandEquippable || Item == RightHandEquippable);

	if (Hand == EHand::Left)
	{
		LeftHandEquippable = nullptr;

		LeftHandMesh->AttachToComponent(LeftHandController, FAttachmentTransformRules::SnapToTargetIncludingScale);
		LeftHandMesh->SetRelativeLocationAndRotation(DefaultLeftHandTransform.Location, DefaultLeftHandTransform.Rotation);
	}
	else
	{
		RightHandEquippable = nullptr;
		
		RightHandMesh->AttachToComponent(RightHandController, FAttachmentTransformRules::SnapToTargetIncludingScale);
		RightHandMesh->SetRelativeLocationAndRotation(DefaultRightHandTransform.Location, DefaultRightHandTransform.Rotation);
	}
}

void AHeroBase::GetDefaultHandMeshLocationAndRotation(const EHand Hand, FVector& Location, FRotator& Rotation) const
{
	const FDefaultHandTransform& HandTransform = (Hand == EHand::Left) ? DefaultLeftHandTransform : DefaultRightHandTransform;

	Location = HandTransform.Location;
	Rotation = HandTransform.Rotation;
}

FVector AHeroBase::GetDefaultHandMeshLocation(const EHand Hand) const
{
	return (Hand == EHand::Left) ? DefaultLeftHandTransform.Location : DefaultRightHandTransform.Location;
}

FRotator AHeroBase::GetDefaultHandMeshRotation(const EHand Hand) const
{
	return (Hand == EHand::Left) ? DefaultLeftHandTransform.Rotation : DefaultRightHandTransform.Rotation;
}

UMeshComponent* AHeroBase::GetBodyAttachmentComponent() const
{
	if (IsLocallyControlled())
	{
		return BodyMesh;
	}
	else
	{
		return GetMesh();
	}
}

USkeletalMeshComponent* AHeroBase::GetHandAttachmentComponent(const EHand Hand) const
{
	return !IsLocallyControlled() ? GetMesh() : (Hand == EHand::Left) ? LeftHandMesh : RightHandMesh;
}

void AHeroBase::PlayHandAnimation(const EHand Hand, const FHandAnim& Anim)
{
	if (IsLocallyControlled())
	{
		if (Hand == EHand::Left)
		{
			if (Anim.DetachedLeft != nullptr)
			{
				LeftHandMesh->PlayAnimation(Anim.DetachedLeft, true);
			}
		}
		else
		{
			if (Anim.DetachedRight != nullptr)
			{
				RightHandMesh->PlayAnimation(Anim.DetachedRight, true);
			}
		}
	}
	else
	{
		UAnimMontage* Montage = (Hand == EHand::Left) ? Anim.FullBodyLeft : Anim.FullBodyRight;
		PlayAnimMontage(Montage);
	}
}

void AHeroBase::StopHandAnimation(const EHand Hand, const FHandAnim& Anim)
{
	if (IsLocallyControlled())
	{
		if (Hand == EHand::Left)
		{
			LeftHandMesh->SetAnimation(nullptr);
		}
		else
		{
			RightHandMesh->SetAnimation(nullptr);
		}
	}
	else
	{
		UAnimMontage* Montage = (Hand == EHand::Left) ? Anim.FullBodyLeft : Anim.FullBodyRight;

		if (Montage != nullptr)
		{
			StopAnimMontage(Montage);
		}
	}
}

UHeroLoadout* AHeroBase::GetLoadout() const
{
	return Loadout;
}

template <EHand Hand>
void AHeroBase::OnEquipPressed()
{
	if (Hand == EHand::Left)
	{
		if (LeftHandEquippable == nullptr)
		{
			Loadout->RequestEquip(LeftHandTrigger, Hand);
		}
		else
		{
			Loadout->RequestUnequip(LeftHandTrigger, LeftHandEquippable);
		}
	}
	else
	{
		if (RightHandEquippable == nullptr)
		{
			Loadout->RequestEquip(RightHandTrigger, Hand);
		}
		else
		{
			Loadout->RequestUnequip(RightHandTrigger, RightHandEquippable);
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

FVector AHeroBase::GetPawnViewLocation() const
{
	return Camera->GetComponentLocation();
}

FRotator AHeroBase::GetViewRotation() const
{
	return Camera->GetComponentRotation();
}