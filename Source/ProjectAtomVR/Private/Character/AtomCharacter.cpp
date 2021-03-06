// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomCharacter.h"

#include "AtomCharacterMovementType.h"
#include "AtomCharacterMovementComponent.h"
#include "NetMotionControllerComponent.h"
#include "HMDCapsuleComponent.h"
#include "HMDCameraComponent.h"
#include "AtomLoadout.h"
#include "Engine/ActorChannel.h"
#include "AtomEquippable.h"
#include "Animation/AnimSequence.h"
#include "WidgetInteractionComponent.h"
#include "Components/SkinnedMeshComponent.h"
#include "AtomPlayerSettings.h"
#include "Components/MeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "AtomPlayerState.h"
#include "AtomGameState.h"
#include "AtomTeamInfo.h"
#include "AtomGameMode.h"

DEFINE_LOG_CATEGORY_STATIC(LogHero, Log, All);

namespace
{
	constexpr float HeadOrientationFactor = 0.35f; // Influence that the head orientation has on the body mesh
	constexpr float HandsOrientationFactor = 1.f - HeadOrientationFactor; // Influence that the direction of hands has on the body mesh.

	constexpr float MeshScaleHeight = 175.f; // Average male height cm. All meshes should be created with this height.
}

// Sets default values
AAtomCharacter::AAtomCharacter(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UAtomCharacterMovementComponent>(ACharacter::CharacterMovementComponentName).SetDefaultSubobjectClass<UHMDCapsuleComponent>(ACharacter::CapsuleComponentName))
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = ETickingGroup::TG_PostPhysics;

	bReplicates = true;
	bReplicateMovement = true;
	bIsDying = false;
	bNetLoadOnClient = false;

	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->bReceivesDecals = false;
	GetMesh()->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCastShadow(false);
	GetMesh()->bPerBoneMotionBlur = false;
	GetMesh()->bUseRefPoseOnInitAnim = true;

	// Setup camera
	Camera = CreateDefaultSubobject<UHMDCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(RootComponent);
	Camera->bLockToHmd = true;
	Camera->SetIsReplicated(true);
	Camera->OnPostNetTransformUpdate.BindUObject(this, &AAtomCharacter::UpdateMeshLocation);

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	BodyMesh->SetOnlyOwnerSee(true);
	BodyMesh->SetCastShadow(false);
	BodyMesh->bAbsoluteLocation = true;
	BodyMesh->bAbsoluteRotation = true;
	BodyMesh->bReceivesDecals = false;

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
	LeftHandMesh->SetCastShadow(false);
	LeftHandMesh->SetupAttachment(LeftHandController);
	LeftHandMesh->SetRelativeLocationAndRotation(FVector{ -20.2, -1.7, 3.3 }, FRotator{ -40, 0, -90 });
	LeftHandMesh->SetIsReplicated(false);
	LeftHandMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	LeftHandMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	LeftHandMesh->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered;

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
	RightHandMesh->SetCastShadow(false);
	RightHandMesh->SetupAttachment(RightHandController);
	RightHandMesh->SetRelativeLocationAndRotation(FVector{ -20.2, 1.7, 3.3 }, FRotator{ -40, 0, 90 });
	RightHandMesh->SetIsReplicated(false);
	RightHandMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	RightHandMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	RightHandMesh->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered;

	// Setup loadout
	Loadout = CreateDefaultSubobject<UAtomLoadout>(TEXT("Loadout"));

	JumpMaxCount = 0;	
}

void AAtomCharacter::BeginPlay()
{
	Super::BeginPlay();

	bool bDelayLoadout = false;
	if (AAtomBaseGameMode* GameMode = GetWorld()->GetAuthGameMode<AAtomBaseGameMode>())
	{
		bDelayLoadout = GameMode->ShouldDelayCharacterLoadoutCreation();
	}

	if (!bDelayLoadout)
	{
		Loadout->SpawnLoadout();
	}	

	if (IsLocallyControlled())
	{
		if (GEngine->HMDDevice.IsValid())
		{
			GEngine->HMDDevice->SetTrackingOrigin(EHMDTrackingOrigin::Floor); // SteamVR and Rift origin is floor
			GEngine->HMDDevice->ResetOrientation();
		}
	}	
}

void AAtomCharacter::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	// Only update locally controlled mesh locations. For remotes and the server, this is updated when the camera
	// receives a transform update through replication, with the adjusted delta time.
	if (IsLocallyControlled())
	{
		UpdateMeshLocation(DeltaTime);
	}	
}

void AAtomCharacter::UpdateMeshLocation(float DeltaTime)
{
	if (bIsDying)
		return;

	const FVector NeckBaseLocation = Camera->GetWorldNeckBaseLocation();
	FVector CameraForward2D = Camera->GetForwardVector();	

	FRotator BodyRotation{ EForceInit::ForceInitToZero };

	// If camera forward is pointing close to +- Z, use existing body forward to prevent popping in random directions with
	// small XY normals.
	if (FMath::Abs(CameraForward2D.Z) > 0.95f)
	{
		Camera->CalculateTorsoPitchAndRoll(BodyRotation);
		CameraForward2D = BodyMesh->GetForwardVector().GetSafeNormal2D();
	}
	else
	{
		// Get torso pitch and roll and invert forward if needed.
		if (Camera->CalculateTorsoPitchAndRoll(BodyRotation))
		{
			CameraForward2D = -CameraForward2D.GetSafeNormal2D();
		}
		else
		{
			CameraForward2D = CameraForward2D.GetSafeNormal2D();
		}
	}

	// Get the averaged controller direction from the two hand controllers
	const FVector RightControllerDirection2D = (RightHandController->GetComponentLocation() - NeckBaseLocation).GetSafeNormal2D();
	const FVector LeftControllerDirection2D = (LeftHandController->GetComponentLocation() - NeckBaseLocation).GetSafeNormal2D();

	FVector ControllerForward2D = ((RightControllerDirection2D + LeftControllerDirection2D) / 2.f).GetSafeNormal2D();

	// If the controller forward is not on the front side of the camera forward, reflect it so that it is.
	const float CameraDotController = FVector::DotProduct(CameraForward2D, ControllerForward2D);
	if (CameraDotController < 0.f)
	{
		ControllerForward2D += 2.f * CameraForward2D;
	}

	const FVector BodyForward2D = CameraForward2D * HeadOrientationFactor + ControllerForward2D * HandsOrientationFactor;

	// Calculate yaw with body forward
	BodyRotation.Yaw = FMath::Acos(BodyForward2D.X) * (180.f / PI);

	if (BodyForward2D.Y < 0.0f)
	{
		BodyRotation.Yaw *= -1.0f;
	}

	// Use rotation to get body location by pivoting on neck socket location
	const FVector NeckBaseSocketOffset = BodyMesh->GetSocketTransform(NeckBaseSocket, RTS_Component).GetTranslation() * BodyMesh->GetComponentScale().Z;
	const FVector WorldNeckOffset = BodyRotation.RotateVector(NeckBaseSocketOffset);
	FVector BodyLocation = NeckBaseLocation - WorldNeckOffset;

	// Calc movement velocity
	RoomScaleVelocity = (BodyLocation - BodyMesh->GetComponentLocation()) / DeltaTime;

	BodyMesh->SetWorldLocationAndRotation(BodyLocation, BodyRotation);

	// Update full body location using only xy for location and yaw rotation
	USkeletalMeshComponent* FullBodyMesh = GetMesh();
	BodyLocation.Z = GetActorLocation().Z;
	FullBodyMesh->SetWorldLocationAndRotation(BodyLocation, FRotator{0, BodyRotation.Yaw, 0});

	// Save new offset for movement component smooth corrections
	const FTransform& FullBodyRelativeTransform = FullBodyMesh->GetRelativeTransform();
	BaseTranslationOffset = FullBodyRelativeTransform.GetLocation();
	BaseRotationOffset = FullBodyRelativeTransform.GetRotation();
}

void AAtomCharacter::SetupPlayerInputComponent(class UInputComponent* InInputComponent)
{
	Super::SetupPlayerInputComponent(InInputComponent);

	// Bind inputs for hero
	InInputComponent->BindAction(TEXT("GripLeft"), EInputEvent::IE_Pressed, this, &AAtomCharacter::OnEquipPressed<EHand::Left>);
	InInputComponent->BindAction(TEXT("GripRight"), EInputEvent::IE_Pressed, this, &AAtomCharacter::OnEquipPressed<EHand::Right>);

	// Setup all movement types component input bindings
	TInlineComponentArray<UAtomCharacterMovementType*> MovementTypeComponents;
	GetComponents(MovementTypeComponents);

	for (auto MovementType : MovementTypeComponents)
	{
		MovementType->SetupPlayerInputComponent(InInputComponent);
	}
}

void SetAndCollectDynamicMaterialInstances(UMeshComponent* MeshComponent, TArray<UMaterialInstanceDynamic*>& MaterialInstances)
{
	for (int32 i = 0; i < MeshComponent->GetNumMaterials(); ++i)
	{
		UMaterialInterface* Material = MeshComponent->GetMaterial(i);
		UMaterialInstanceDynamic** MaterialInstance = MaterialInstances.FindByPredicate([Material](UMaterialInstanceDynamic* MI)
		{
			return MI->Parent == Material;
		});

		if (MaterialInstance)
		{
			MeshComponent->SetMaterial(i, *MaterialInstance);
		}
		else
		{
			MaterialInstances.Add(MeshComponent->CreateAndSetMaterialInstanceDynamic(i));
		}
	}
}

void AAtomCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();			

	DefaultLeftHandTransform.Location = LeftHandMesh->RelativeLocation;
	DefaultLeftHandTransform.Rotation = LeftHandMesh->RelativeRotation;

	DefaultRightHandTransform.Location = RightHandMesh->RelativeLocation;
	DefaultRightHandTransform.Rotation = RightHandMesh->RelativeRotation;

	Loadout->InitializeLoadout(this);

	// Try to reuse any material instances on other meshes with same material
	SetAndCollectDynamicMaterialInstances(GetMesh(), MeshMaterialInstances);
	SetAndCollectDynamicMaterialInstances(BodyMesh, MeshMaterialInstances);
	SetAndCollectDynamicMaterialInstances(LeftHandMesh, MeshMaterialInstances);
	SetAndCollectDynamicMaterialInstances(RightHandMesh, MeshMaterialInstances);
}

void AAtomCharacter::PostNetReceiveLocationAndRotation()
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

FVector AAtomCharacter::GetVelocity() const
{
	return GetCapsuleComponent()->GetComponentVelocity();
}

bool AAtomCharacter::ReplicateSubobjects(UActorChannel *Channel, FOutBunch *Bunch, FReplicationFlags *RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	bWroteSomething |= Channel->ReplicateSubobject(Loadout, *Bunch, *RepFlags);

	return bWroteSomething;
}

void AAtomCharacter::Destroyed()
{
	Loadout->DestroyLoadout();

	if (RightHandEquippable)
	{
		RightHandEquippable->Destroy();
		RightHandEquippable = nullptr;
	}

	if (LeftHandEquippable)
	{
		LeftHandEquippable->Destroy();
		LeftHandEquippable = nullptr;
	}

	Super::Destroyed();
}

void AAtomCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	NotifyTeamChanged();
}

bool AAtomCharacter::ShouldTakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const
{
	bool bCanDamage = false;

	if (AAtomGameMode* GameMode = GetWorld()->GetAuthGameMode<AAtomGameMode>())
	{
		bCanDamage = GameMode->CanDamage(EventInstigator, GetController());
	}
	
	return bCanDamage && CanDie() && Super::ShouldTakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
}

float AAtomCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (Health > 0)
	{
		if (AAtomGameMode* GameMode = GetWorld()->GetAuthGameMode<AAtomGameMode>())
		{
			Damage = GameMode->ModifyDamage(Damage, DamageEvent, EventInstigator, GetController());
		}

		Damage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

		//TakeHit(ActualDamage, DamageEvent, EventInstigator, DamageCauser);

		if (Damage > 0)
		{
			Health -= Damage;

			if (Health <= 0)
			{
				Die(EventInstigator);
			}
		}
	}

	return Damage;
}

void AAtomCharacter::Die(AController* Killer)
{
	check(HasAuthority());

	bIsDying = true;
	TearOff();

	AAtomGameMode* GameMode = Cast<AAtomGameMode>(GetWorld()->GetAuthGameMode());
	check(GameMode && "Deaths should only happen on AtomGameMode");

	GameMode->RegisterKill(Killer, GetController());

	DetachFromControllerPendingDestroy();

	if (LeftHandEquippable != nullptr)
	{
		LeftHandEquippable->Drop();
	}

	if (RightHandEquippable != nullptr)
	{
		RightHandEquippable->Drop();
	}

	OnDeath();
}

void AAtomCharacter::OnDeath()
{
	Loadout->OnCharacterControllerChanged();
	Loadout->DisableLoadout();

	// Stop any existing montages
	StopAnimMontage();
	
	// Set collision properties
	static FName RagdollProfile{ TEXT("Ragdoll") };
	GetMesh()->SetCollisionProfileName(RagdollProfile);

	SetActorEnableCollision(true);

	// Activate ragdoll
	if (GetMesh()->GetPhysicsAsset())
	{
		// Set all bodies of the mesh component to simulate
		GetMesh()->SetAllBodiesSimulatePhysics(true);
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->WakeAllRigidBodies();

		GetMesh()->bBlendPhysics = true;
	}

	// Stop and disable any character movement
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->SetComponentTickEnabled(false);

	// Disable capsule
	GetCapsuleComponent()->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

	SetLifeSpan(10.0f);
}

void AAtomCharacter::OnReceivedDamage()
{

}

void AAtomCharacter::OnRep_IsDying()
{
	OnDeath();
}

void AAtomCharacter::OnRep_IsRightHanded()
{

}

void AAtomCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	Loadout->OnCharacterControllerChanged();
	NotifyTeamChanged();

	if (RightHandEquippable != nullptr)
	{
		RightHandEquippable->UpdateCharacterAttachment();
	}

	if (LeftHandEquippable != nullptr)
	{
		LeftHandEquippable->UpdateCharacterAttachment();
	}
}

void AAtomCharacter::UnPossessed()
{
	Super::UnPossessed();

	Loadout->OnCharacterControllerChanged();

	if (RightHandEquippable)
	{
		RightHandEquippable->UpdateCharacterAttachment();
	}

	if (LeftHandEquippable)
	{
		LeftHandEquippable->UpdateCharacterAttachment();
	}
}

void AAtomCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAtomCharacter, bIsDying);
	DOREPLIFETIME_CONDITION(AAtomCharacter, bIsRightHanded, COND_SkipOwner);
}

void AAtomCharacter::MovementTeleport(const FVector& DestLocation, const FRotator& DestRotation)
{
	if (!Controller || !Controller->IsMoveInputIgnored())
	{
		// Get correct location and rotation
		FVector DestinationOffset = GetActorLocation() - Camera->GetWorldHeadLocation();
		DestinationOffset.Z = 0.f;
		const FVector CapsuleDestination = DestLocation + DestinationOffset;

		// Just teleport if server call and not locally controlled
		GetHeroMovementComponent()->TeleportMove(CapsuleDestination);
	}
}

UHMDCapsuleComponent* AAtomCharacter::GetHMDCapsuleComponent() const
{
	return CastChecked<UHMDCapsuleComponent>(GetCapsuleComponent());
}

void AAtomCharacter::ApplyPlayerSettings(const FAtomPlayerSettings& PlayerSettings)
{
	bIsRightHanded = PlayerSettings.bIsRightHanded;

	const FVector MeshScale{ PlayerSettings.PlayerHeight / MeshScaleHeight };
	GetMesh()->SetWorldScale3D(MeshScale);
	BodyMesh->SetWorldScale3D(MeshScale);

	const FAtomCharacterSettings* CharacterSettings = PlayerSettings.CharacterSettings.FindByPredicate(
		[this](const FAtomCharacterSettings& Settings) 
	{
		return this->GetClass() == Settings.Character;
	});

	if (CharacterSettings)
	{
		Loadout->SetLoadoutOffset(CharacterSettings->LoadoutOffset);
	}	
}

USceneComponent* AAtomCharacter::GetHandMeshTarget(const EHand Hand) const
{
	return (Hand == EHand::Left) ? LeftHandMesh : RightHandMesh;
}

void AAtomCharacter::Equip(AAtomEquippable* Item, const EHand Hand)
{ 
	// Unequip first if hand is in use
	AAtomEquippable*& EquippablePtr = (Hand == EHand::Left) ? LeftHandEquippable : RightHandEquippable;

	if (EquippablePtr != nullptr)
	{
		EquippablePtr->Unequip();
	}

	Item->Equip(Hand);
}

void AAtomCharacter::OnEquipped(AAtomEquippable* Item, const EHand Hand)
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

void AAtomCharacter::Unequip(AAtomEquippable* Item, const EHand Hand)
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

void AAtomCharacter::OnUnequipped(AAtomEquippable* Item, const EHand Hand)
{
	ensure(Item == LeftHandEquippable || Item == RightHandEquippable);

	if (Hand == EHand::Left)
	{
		LeftHandEquippable = nullptr;

		LeftHandMesh->AttachToComponent(LeftHandController, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		LeftHandMesh->SetRelativeLocationAndRotation(DefaultLeftHandTransform.Location, DefaultLeftHandTransform.Rotation);
	}
	else
	{
		RightHandEquippable = nullptr;
		
		RightHandMesh->AttachToComponent(RightHandController, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		RightHandMesh->SetRelativeLocationAndRotation(DefaultRightHandTransform.Location, DefaultRightHandTransform.Rotation);
	}

	// Only if primary equipped hand
	if (Item->GetEquippedHand() == Hand && Loadout->IsInLoadout(Item))
	{		
		Loadout->ReturnToLoadout(Item);		
	}
}

void AAtomCharacter::DiscardFromLoadout(AAtomEquippable* Item)
{
	if (Loadout->IsInLoadout(Item))
	{
		Loadout->DiscardFromLoadout(Item);
	}
}

void AAtomCharacter::GetDefaultHandMeshLocationAndRotation(const EHand Hand, FVector& Location, FRotator& Rotation) const
{
	const FDefaultHandTransform& HandTransform = (Hand == EHand::Left) ? DefaultLeftHandTransform : DefaultRightHandTransform;

	Location = HandTransform.Location;
	Rotation = HandTransform.Rotation;
}

FVector AAtomCharacter::GetDefaultHandMeshLocation(const EHand Hand) const
{
	return (Hand == EHand::Left) ? DefaultLeftHandTransform.Location : DefaultRightHandTransform.Location;
}

FRotator AAtomCharacter::GetDefaultHandMeshRotation(const EHand Hand) const
{
	return (Hand == EHand::Left) ? DefaultLeftHandTransform.Rotation : DefaultRightHandTransform.Rotation;
}

UMeshComponent* AAtomCharacter::GetBodyAttachmentComponent() const
{
	if (IsLocallyControlled() && !bIsDying)
	{
		return BodyMesh;
	}
	else
	{
		return GetMesh();
	}
}

USkeletalMeshComponent* AAtomCharacter::GetHandAttachmentComponent(const EHand Hand) const
{
	return (!IsLocallyControlled() && !bIsDying) ? GetMesh() : (Hand == EHand::Left) ? LeftHandMesh : RightHandMesh;
}

FVector AAtomCharacter::GetRoomScaleVelocity() const
{
	return RoomScaleVelocity;
}

void AAtomCharacter::PlayHandAnimation(const EHand Hand, const FHandAnim& Anim)
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

void AAtomCharacter::StopHandAnimation(const EHand Hand, const FHandAnim& Anim)
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

UAtomLoadout* AAtomCharacter::GetLoadout() const
{
	return Loadout;
}

bool AAtomCharacter::CanDie() const
{
	return Health > 0 && !IsPendingKill();
}

void AAtomCharacter::NotifyTeamChanged()
{
	AAtomPlayerState* AtomPlayerState = Cast<AAtomPlayerState>(PlayerState);

	if (AtomPlayerState && AtomPlayerState->GetTeam())
	{
		static const FName TeamColorMaterialParam = TEXT("TeamColor");

		for (auto MI : MeshMaterialInstances)
		{
			MI->SetVectorParameterValue(TeamColorMaterialParam, AtomPlayerState->GetTeam()->TeamColor);
		}
	}
}

template <EHand Hand>
void AAtomCharacter::OnEquipPressed()
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

FVector AAtomCharacter::GetPawnViewLocation() const
{
	return Camera->GetComponentLocation();
}

FRotator AAtomCharacter::GetViewRotation() const
{
	return Camera->GetComponentRotation();
}
