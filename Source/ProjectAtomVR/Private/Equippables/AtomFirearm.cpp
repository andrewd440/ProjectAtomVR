// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomFirearm.h"
#include "AtomCharacter.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimInstance.h"
#include "AnimNotify_Firearm.h"
#include "EquippableStateActiveFirearm.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "EquippableStateFiring.h"
#include "MagazineAmmoLoader.h"
#include "Engine/ActorChannel.h"

DEFINE_LOG_CATEGORY_STATIC(LogFirearm, Log, All);

namespace
{
	static const FName CartridgeFiredEmitterName{ TEXT("CartridgeFired") };
	static const FName CartridgeUnfiredEmitterName{ TEXT("CartridgeUnfired") };
	static const FName CartridgeAttachSocket{ TEXT("CartridgeAttach") };
	static const FName HandGripSocket{ TEXT("Grip") };
	static const FName ChamberingHandleSocket{ TEXT("ChamberingHandle") };
	static const FName MuzzleSocket{ TEXT("Muzzle") };
	static const FName SlideLockSection{ TEXT("SlideLock") };

	// Tolerances for snapping back to original location once the recoil has returned.
	constexpr float ActiveRecoilRotationTolerance = 0.001f;
	constexpr float ActiveReciolOffsetTolerance = 0.1f;
}

AAtomFirearm::AAtomFirearm(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<USkeletalMeshComponent>(AAtomEquippable::MeshComponentName).
							  SetDefaultSubobjectClass<UEquippableStateActiveFirearm>(AAtomEquippable::ActiveStateName))
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	bReplicates = true;

	LoadoutType = ELoadoutType::Weapon;

	Stats.Damage = 20;
	Stats.FireRate = 0.1f;
	Stats.RecoilDampening = .5f;
	Stats.RecoilDirectionalPush = FVector2D{ -1.f, 0.f };
	Stats.RecoilRotationalPush = FVector2D{ 0.f, -1.f };
	Stats.RecoilPushSpread = 30.f;

	RecoilVelocity.Directional = FVector::ZeroVector;
	RecoilVelocity.Angular = FVector::ZeroVector;
	bIsRecoilActive = false;

	ChamberingHandleRadius = 10.f;
	bIsChamberEmpty = false;
	bIsSlideLockActive = false;
	bIsHoldingChamberHandle = false;

	GetMesh<USkeletalMeshComponent>()->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered;
	GetMesh()->SetCastShadow(false);

	FiringState = CreateDefaultSubobject<UEquippableStateFiring>(TEXT("FiringState"));

	CartridgeMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CartridgeMesh"));
	CartridgeMeshComponent->SetupAttachment(GetMesh(), CartridgeAttachSocket);
	CartridgeMeshComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	CartridgeMeshComponent->SetIsReplicated(false);
	CartridgeMeshComponent->SetCastShadow(false);
}


void AAtomFirearm::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	UpdateChamberingHandle();

	if (bIsRecoilActive)
	{
		UpdateRecoilOffset(DeltaTime);
	}
	
}

void AAtomFirearm::UpdateChamberingHandle()
{
	if (bIsHoldingChamberHandle)
	{
		if (CanGripChamberingHandle())
		{
			check(ChamberingIndex < ChamberHandleMovement.Num());

			const USceneComponent* const Hand = GetHeroOwner()->GetHandTrigger(!EquipStatus.Hand);
			const FVector HandLocation = ActorToWorld().InverseTransformPosition(Hand->GetComponentLocation());

			// Get the current movement progress based on the project of the hand delta on the chamber movement vector
			const FVector& RequiredChamberMovement = ChamberHandleMovement[ChamberingIndex];
			const FVector HandDelta = HandLocation - ChamberingHandStartLocation;
			float NewProgress = FVector::DotProduct(RequiredChamberMovement, 
				HandDelta.ProjectOnTo(RequiredChamberMovement)) / FMath::Square(RequiredChamberMovement.Size());

			if (NewProgress >= 1.f)
			{
				// Play chambering sound if we hit a new checkpoint
				if (ChamberingProgress < 1.f &&
					ChamberingSounds.IsValidIndex(ChamberingIndex) &&
					ChamberingSounds[ChamberingIndex])
				{
					UGameplayStatics::SpawnSoundAttached(ChamberingSounds[ChamberingIndex], GetMesh(), ChamberingHandleSocket);
				}

				if (ChamberHandleMovement.Num() > ChamberingIndex + 1)
				{
					// Move to the next movement if we have one
					++ChamberingIndex;
					NewProgress = 0.f;
				}
				else
				{
					NewProgress = 1.f;

					if (LastChamberState == EChamberState::Set)
					{
						ReloadChamber(false);
						LastChamberState = EChamberState::Unset;
					}
				}
			}
			else if (NewProgress <= 0.f)
			{
				// Play chambering sound if we hit a new checkpoint
				const int32 ChamberingSoundIndex = ChamberHandleMovement.Num() * 2 - ChamberingIndex - 1; // Invert when playing back
				if (ChamberingProgress > 0.f &&
					ChamberingSounds.IsValidIndex(ChamberingSoundIndex) &&
					ChamberingSounds[ChamberingSoundIndex])
				{
					UGameplayStatics::SpawnSoundAttached(ChamberingSounds[ChamberingSoundIndex], GetMesh(), ChamberingHandleSocket);
				}

				if (ChamberingIndex > 0)
				{
					// Move to the last movement if we have one
					--ChamberingIndex;
					NewProgress = 1.f;
				}
				else
				{
					NewProgress = 0.f;
					LastChamberState = EChamberState::Set;
				}
			}

			ChamberingProgress = NewProgress;
		}
		else
		{
			OnChamberingHandleReleased();
		}
	}
}

void AAtomFirearm::UpdateRecoilOffset(float DeltaSeconds)
{
	UE_LOG(LogFirearm, Log, TEXT("Updating recoil offset for firearm."));

	USceneComponent* MyMesh = GetMesh();
	USceneComponent* OffsetTarget = GetOffsetTarget();

	const FTransform ToOffsetTargetTransform = MyMesh->ComponentToWorld.GetRelativeTransform(OffsetTarget->ComponentToWorld);

	// Get the move and rotation delta in local space
	const FVector AngularVelocityLocal = RecoilVelocity.Angular * DeltaSeconds;

	const FQuat RotationDeltaLocal{ ToOffsetTargetTransform.TransformVector(AngularVelocityLocal.GetSafeNormal()), AngularVelocityLocal.Size() };
	const FVector DirectionalDeltaLocal = ToOffsetTargetTransform.TransformVector(RecoilVelocity.Directional);

	OffsetTarget->AddLocalTransform(FTransform{ RotationDeltaLocal, DirectionalDeltaLocal, OffsetTarget->RelativeScale3D });

	// Apply force to return to original location/rotation
	FVector OriginalRelativeLocation;
	FRotator OriginalRelativeRotation;
	GetOriginalOffsetTargetLocationAndRotation(OriginalRelativeLocation, OriginalRelativeRotation);

	const FTransform OffsetTargetRelativeTransform = OffsetTarget->GetRelativeTransform();
	
	// Rotation delta needed to get to original rotation
	FQuat ToOriginalRotation = OriginalRelativeRotation.Quaternion() * OffsetTargetRelativeTransform.GetRotation().Inverse();

	FVector ToOriginalAxis; float ToOriginalAngle;
	ToOriginalRotation.ToAxisAndAngle(ToOriginalAxis, ToOriginalAngle);

	FQuat OffsetTargetRelativeToMeshRotation = (OffsetTargetRelativeTransform.GetRotation() * ToOffsetTargetTransform.GetRotation()).Inverse();

	// Get movement needed to get to original location	
	FVector ToOriginalLocation = OffsetTargetRelativeToMeshRotation.RotateVector(OriginalRelativeLocation - OffsetTargetRelativeTransform.GetTranslation());
	const float DistanceToOriginalLocation = ToOriginalLocation.Size();

	if (ToOriginalAngle < ActiveRecoilRotationTolerance && 
		DistanceToOriginalLocation < ActiveReciolOffsetTolerance)
	{
		// If close enough to original, set it back
		OffsetTarget->SetRelativeLocationAndRotation(OriginalRelativeLocation, OriginalRelativeRotation);
		bIsRecoilActive = false;
	}
	else
	{
		// Apply to angular velocity		
		ToOriginalAxis = OffsetTargetRelativeToMeshRotation.RotateVector(ToOriginalAxis);

		RecoilVelocity.Angular *= Stats.RecoilDampening;
		RecoilVelocity.Angular += ToOriginalAxis * FMath::Min(ToOriginalAngle, Stats.Stability * DeltaSeconds);

		// Apply directional velocity to move to original location
		RecoilVelocity.Directional *= Stats.RecoilDampening;
		RecoilVelocity.Directional += ToOriginalLocation.GetSafeNormal() * 
			FMath::Min(DistanceToOriginalLocation, Stats.Stability * DeltaSeconds);
	}
}

FVector AAtomFirearm::GetMuzzleLocation() const
{
	return GetMesh()->GetSocketLocation(MuzzleSocket);
}

FQuat AAtomFirearm::GetMuzzleRotation() const
{
	return GetMesh()->GetSocketQuaternion(MuzzleSocket);
}

bool AAtomFirearm::IsChamberEmpty() const
{
	return bIsChamberEmpty;
}

float AAtomFirearm::GetChamberingProgress() const
{
	return (ChamberingProgress + ChamberingIndex) / (float)ChamberHandleMovement.Num();
}

bool AAtomFirearm::IsHoldingChamberingHandle() const
{
	return bIsHoldingChamberHandle;
}

bool AAtomFirearm::CanFire() const
{
	return !IsChamberEmpty() &&
		!bIsHoldingChamberHandle &&
		!bIsSlideLockActive	&&
		!IsMuzzleInGeometry();
}

bool AAtomFirearm::IsMuzzleInGeometry() const
{
	const FCollisionObjectQueryParams ObjectParams{ FCollisionObjectQueryParams::AllStaticObjects };
	const FCollisionQueryParams QueryParams{ NAME_None, false, this };

	const FVector WorldLocation = GetTransform().TransformPosition(BlockFireVolume.RelativePosition);
	const FQuat WorldRotation = GetTransform().GetRotation() * BlockFireVolume.RelativeRotation;

	//DrawDebugCapsule(GetWorld(), WorldLocation, BlockFireVolume.CapsuleHalfHeight, BlockFireVolume.CapsuleRadius, 
	// WorldRotation, FColor::Blue, false, 1.f, 0, .2f);

	const FCollisionShape MuzzleCollision = FCollisionShape::MakeCapsule(BlockFireVolume.CapsuleRadius, BlockFireVolume.CapsuleHalfHeight);
	return GetWorld()->OverlapAnyTestByObjectType(WorldLocation, WorldRotation, ObjectParams, MuzzleCollision, QueryParams);
}

void AAtomFirearm::LoadAmmo(UObject* LoadObject, bool bForceLocalOnly)
{
	if (!bForceLocalOnly && 
		!HasAuthority() && 
		GetHeroOwner() && // Player will not be able to reload quicker than owner is replicated, so guard here.
		GetHeroOwner()->IsLocallyControlled())
	{
		ServerLoadAmmo(LoadObject);
	}

	AmmoLoader->LoadAmmo(LoadObject);	

	if (LoadAmmoSound)
	{
		UGameplayStatics::SpawnSoundAttached(LoadAmmoSound, GetMesh()); // #AtomTodo Use custom loading socket
	}
}

void AAtomFirearm::DiscardAmmo()
{
	if (AmmoLoader->DiscardAmmo())
	{
		// Only replicate if successful
		if (GetHeroOwner() && GetHeroOwner()->IsLocallyControlled() && !HasAuthority())
		{
			ServerDiscardAmmo();
		}
		
		if (DiscardAmmoSound)
		{
			UGameplayStatics::SpawnSoundAttached(DiscardAmmoSound, GetMesh()); // #AtomTodo Use custom loading socket
		}
	}
}

void AAtomFirearm::ServerLoadAmmo_Implementation(UObject* LoadObject)
{
	LoadAmmo(LoadObject);
}

bool AAtomFirearm::ServerLoadAmmo_Validate(UObject* LoadObject)
{
	return true;
}

void AAtomFirearm::ServerDiscardAmmo_Implementation()
{
	DiscardAmmo();
}

bool AAtomFirearm::ServerDiscardAmmo_Validate()
{
	return true;
}

void AAtomFirearm::FireShot()
{
	check(ShotType);

	UE_LOG(LogFirearm, Log, TEXT("FireShot by %s"), HasAuthority() ? TEXT("Authority") : TEXT("Client"));

	// Since the firing state will be call FireShot while active, it will be
	// call by Autonomous, Authority, and Simulated connections. We will allow simulated
	// proxies to simulate fire. Autonomous will simulate and call ServerFire. Authority
	// will only fire here if it is locally controlled. Otherwise, it will wait on the controlling
	// client to sent a fire event through ServerFireShot.
	if (GetHeroOwner()->IsLocallyControlled())
	{
		// Get shot data before applying recoil
		const FShotData ShotData = ShotType->GetShotData();
		GenerateShotRecoil(ShotData.Seed);

		// Play effects and reload chamber before firing the shot. This keeps situations such as killing yourself in order.
		// i.e. FireShot -> Die -> Drop Firearm -> Pop FiringState (which calls StopFiringSequence)
		PlaySingleShotSequence();
		ReloadChamber(true);

		if (HasAuthority())
		{			
			ShotType->FireShot(ShotData);
		}
		else
		{
			ShotType->SimulateShot(ShotData);
			ServerFireShot(ShotData);
		}
	}
	else if(Role == ENetRole::ROLE_SimulatedProxy)
	{
		// Get shot data before applying recoil
		const FShotData ShotData = ShotType->GetShotData();
		GenerateShotRecoil(ShotData.Seed);

		// See above comment for operation ordering
		PlaySingleShotSequence();
		ReloadChamber(true);

		ShotType->SimulateShot(ShotData);
	}
}

void AAtomFirearm::DryFire()
{
	if (DryFireSound)
	{
		UGameplayStatics::SpawnSoundAttached(DryFireSound, GetMesh(), CartridgeAttachSocket);
	}	
}

void AAtomFirearm::ServerFireShot_Implementation(FShotData ShotData)
{
	UE_LOG(LogFirearm, Log, TEXT("ServerFireShot"));
	GenerateShotRecoil(ShotData.Seed);
	ShotType->FireShot(ShotData);

	if (GetNetMode() != ENetMode::NM_DedicatedServer)
	{
		PlaySingleShotSequence();
	}

	ReloadChamber(true);
}

bool AAtomFirearm::ServerFireShot_Validate(FShotData ShotData)
{
	return true;
}

void AAtomFirearm::PlaySingleShotSequence()
{
	// Activate if not active or not looping
	if (MuzzleFX != nullptr && (MuzzleFXComponent == nullptr || !MuzzleFX->IsLooping()))
	{
		MuzzleFXComponent = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, GetMesh(), MuzzleSocket);
		MuzzleFXComponent->ActivateSystem();
	}

	CartridgeMeshComponent->SetVisibility(true);
	CartridgeMeshComponent->SetStaticMesh(CartridgeFiredMesh);

	if (FireSound && (!FireSound->IsLooping() || !FireSoundComponent))
	{
		FireSoundComponent = UGameplayStatics::SpawnSoundAttached(FireSound, GetMesh(), MuzzleSocket);
		FireSoundComponent->Play();
	}

	if (FiringMontage)
	{
		const float MontageLength = FiringMontage->CalculateSequenceLength();
		GetMesh<USkeletalMeshComponent>()->GetAnimInstance()->Montage_Play(FiringMontage, MontageLength / Stats.FireRate);
	}
}

void AAtomFirearm::OnOppositeHandTriggerPressed()
{
	if (CanGripChamberingHandle())
	{		
		OnChamberingHandleGrabbed();
		ServerSetIsHoldingChamberingHandle(true);
	}
}

void AAtomFirearm::OnOppositeHandTriggerReleased()
{
	if (bIsHoldingChamberHandle)
	{
		OnChamberingHandleReleased();
		ServerSetIsHoldingChamberingHandle(false);
	}	
}

bool AAtomFirearm::CanGripChamberingHandle() const
{
	if (!bIsSlideLockActive && GetHeroOwner()->GetEquippable(!EquipStatus.Hand) == nullptr)
	{		
		const FVector HandleLocation = GetMesh()->GetSocketLocation(ChamberingHandleSocket);
		const FVector GripLocation = GetHeroOwner()->GetHandTrigger(!EquipStatus.Hand)->GetComponentLocation();
		if (FVector::DistSquared(GripLocation, HandleLocation) <= ChamberingHandleRadius * ChamberingHandleRadius)
		{
			return true;
		}
	}

	return false;
}

void AAtomFirearm::OnChamberingHandleGrabbed()
{
	bIsHoldingChamberHandle = true;
	ChamberingIndex = 0;
	LastChamberState = EChamberState::Set;

	// Assign relative hand location
	const USceneComponent* const Hand = GetHeroOwner()->GetHandTrigger(!EquipStatus.Hand);
	ChamberingHandStartLocation = ActorToWorld().InverseTransformPosition(Hand->GetComponentLocation());
}

void AAtomFirearm::OnChamberingHandleReleased()
{
	bIsHoldingChamberHandle = false;

	if (!bIsSlideLockActive && ChamberingProgress > 0.2f) // If has enough progress to make a sound
	{
		const int32 LastChamberingSoundIndex = ChamberHandleMovement.Num() * 2 - 1;
		if (ChamberingSounds.IsValidIndex(LastChamberingSoundIndex) && ChamberingSounds[LastChamberingSoundIndex])
		{
			UGameplayStatics::SpawnSoundAttached(ChamberingSounds[LastChamberingSoundIndex], GetMesh(), ChamberingHandleSocket);
		}
	}

	// Reset chambering params
	ChamberingProgress = 0;
	ChamberingIndex = 0;
	LastChamberState = EChamberState::Set;
}

void AAtomFirearm::OnSlideLockPressed()
{
	if (bIsSlideLockActive && AmmoLoader->GetAmmoCount() > 0)
	{
		ReleaseSlideLock();
		ServerSetSlideLock(false);
	}	
}

void AAtomFirearm::ActivateSlideLock()
{
	if (FiringMontage && FiringMontage->IsValidSectionName(SlideLockSection))
	{
		bIsSlideLockActive = true;

		if (bIsHoldingChamberHandle)
		{
			OnChamberingHandleReleased();
		}

		auto AnimInstance = GetMesh<USkeletalMeshComponent>()->GetAnimInstance();
		AnimInstance->Montage_Play(FiringMontage);
		AnimInstance->Montage_JumpToSection(SlideLockSection, FiringMontage);
		AnimInstance->Montage_Pause(FiringMontage);		
	}
	else
	{
		UE_LOG(LogFirearm, Warning, TEXT("No SlideLock section found on FiringMontage. Disabling slide lock for %s"), *GetName());
	}
}

void AAtomFirearm::ReleaseSlideLock()
{
	bIsSlideLockActive = false;

	// Play the last chambering sound if valid
	const int32 LastChamberingSoundIndex = ChamberHandleMovement.Num() * 2 - 1;
	if (ChamberingSounds.IsValidIndex(LastChamberingSoundIndex) && ChamberingSounds[LastChamberingSoundIndex])
	{
		UGameplayStatics::SpawnSoundAttached(ChamberingSounds[LastChamberingSoundIndex], GetMesh(), ChamberingHandleSocket);
	}

	GetMesh<USkeletalMeshComponent>()->GetAnimInstance()->Montage_Resume(FiringMontage);
	ReloadChamber(false);
}

void AAtomFirearm::GenerateShotRecoil(uint8 Seed)
{
	// Get random rotation [-1, 1] to factor in with RecoilPushSpread and
	// apply the rotation to RecoilPush
	FMath::RandInit(Seed);
	const float Rotation = FMath::FRandRange(-1.f, 1.f) * Stats.RecoilPushSpread;
	const FVector2D RecoilInput = Stats.RecoilRotationalPush.GetRotated(Rotation);
	
	// Apply recoil impulses
	RecoilVelocity.Angular = FVector::CrossProduct(FVector{ 0.f, RecoilInput.X, RecoilInput.Y}, FVector::ForwardVector);
	if (IsSecondaryHandAttached())
	{
		RecoilVelocity.Angular *= Stats.RecoilDampening;
	}

	RecoilVelocity.Directional = FVector{ Stats.RecoilDirectionalPush.X, 0.f, Stats.RecoilDirectionalPush.Y };
	RecoilVelocity.Directional *= FMath::FRandRange(0.5f, 1.f);

	bIsRecoilActive = true;
}

void AAtomFirearm::ReloadChamber(bool bIsFired)
{
	// Eject first if not empty
	if (!bIsChamberEmpty && CartridgeEjectComponent)
	{
		CartridgeEjectComponent->Activate(true);
		CartridgeEjectComponent->SetEmitterEnable(CartridgeFiredEmitterName, bIsFired);
		CartridgeEjectComponent->SetEmitterEnable(CartridgeUnfiredEmitterName, !bIsFired);
	}

	// Now update the ammo loader and chamber
	if (AmmoLoader->GetAmmoCount() <= 0)
	{
		bIsChamberEmpty = true;

		CartridgeMeshComponent->SetVisibility(false);

		if (Stats.bHasSlideLock)
		{
			ActivateSlideLock();
		}

		AmmoLoader->OnAmmoCountChanged.ExecuteIfBound(); // Ammo count changed, but not from AmmoLoader
	}
	else
	{		
		bIsChamberEmpty = false;
		AmmoLoader->ConsumeAmmo();

		CartridgeMeshComponent->SetVisibility(true);
		CartridgeMeshComponent->SetStaticMesh(CartridgeUnfiredMesh);
	}	

	UE_LOG(LogFirearm, Log, TEXT("ReloadChamber by %s with IsChamberEmpty = %d and ammo count = %d"), 
		HasAuthority() ? TEXT("Authority") : TEXT("Client"), bIsChamberEmpty, AmmoLoader->GetAmmoCount());
}

void AAtomFirearm::OnEjectedCartridgeCollide(FName EventName, float EmitterTime, int32 ParticleTime, FVector Location, 
	FVector Velocity, FVector Direction, FVector Normal, FName BoneName, UPhysicalMaterial* PhysMat)
{
	UGameplayStatics::SpawnSoundAtLocation(GetWorld(), CartridgeCollideSound, Location);
}

void AAtomFirearm::OnRep_IsHoldingChamberHandle()
{
	if (bIsHoldingChamberHandle)
	{
		OnChamberingHandleGrabbed();
	}
	else
	{
		OnChamberingHandleReleased();
	}
}

void AAtomFirearm::OnRep_IsSlideLockActive()
{
	if(!bIsSlideLockActive)
	{
		ReleaseSlideLock();
	}
	else if(!HasActorBegunPlay()) // Only activate on initial rep, otherwise will be automatically activated on remotes.
	{
		ActivateSlideLock();
	}
	else
	{
		bIsSlideLockActive = false; // During play, only allow rep to set this for releases
	}
}

void AAtomFirearm::SetupInputComponent(UInputComponent* InInputComponent)
{
	Super::SetupInputComponent(InInputComponent);

	const FName WeaponInteractName = (EquipStatus.Hand == EHand::Right) ? TEXT("WeaponInteractLeft") : TEXT("WeaponInteractRight");
	InInputComponent->BindAction(WeaponInteractName, IE_Pressed, this, &AAtomFirearm::OnOppositeHandTriggerPressed).bConsumeInput = false;
	InInputComponent->BindAction(WeaponInteractName, IE_Released, this, &AAtomFirearm::OnOppositeHandTriggerReleased).bConsumeInput = false;

	if (Stats.bHasSlideLock)
	{
		const FName SlideLockName = (EquipStatus.Hand == EHand::Right) ? TEXT("SlideLockRight") : TEXT("SlideLockLeft");
		InInputComponent->BindAction(SlideLockName, IE_Pressed, this, &AAtomFirearm::OnSlideLockPressed);
	}

	AmmoLoader->SetupInputComponent(InInputComponent);
}

void AAtomFirearm::ServerSetSlideLock_Implementation(bool bIsActive)
{
	if (bIsActive != bIsSlideLockActive)
	{
		if (bIsActive)
		{
			ActivateSlideLock();
		}
		else
		{
			ReleaseSlideLock();
		}
	}
}

bool AAtomFirearm::ServerSetSlideLock_Validate(bool bIsActive)
{
	return true;
}

void AAtomFirearm::ServerSetIsHoldingChamberingHandle_Implementation(bool bIsHeld)
{
	if (bIsHeld != bIsHoldingChamberHandle)
	{
		if (bIsHeld)
		{
			OnChamberingHandleGrabbed();
		}
		else
		{
			OnChamberingHandleReleased();
		}
	}
}

bool AAtomFirearm::ServerSetIsHoldingChamberingHandle_Validate(bool bIsHeld)
{
	return true;
}

void AAtomFirearm::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AAtomFirearm, bIsHoldingChamberHandle, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AAtomFirearm, bIsSlideLockActive, COND_SkipOwner);
}

void AAtomFirearm::StartFiringSequence()
{
	if (TriggerPullMontage)
	{
		GetMesh<USkeletalMeshComponent>()->GetAnimInstance()->Montage_Play(TriggerPullMontage);
	}
}

void AAtomFirearm::StopFiringSequence()
{
	if (MuzzleFXComponent && MuzzleFXComponent->IsActive() && MuzzleFXComponent->Template->IsLooping())
	{
		MuzzleFXComponent->DeactivateSystem();
		MuzzleFXComponent = nullptr;
	}

	if (FireSoundComponent && FireSound->IsLooping() && FireSoundComponent->IsPlaying())
	{
		FireSoundComponent->FadeOut(1.0f, 1.0f); // Allow SoundNodeFadeOut nodes to play tail fire sound
		FireSoundComponent = nullptr;
	}

	if (TriggerPullMontage)
	{
		GetMesh<USkeletalMeshComponent>()->GetAnimInstance()->Montage_Stop(.1f, TriggerPullMontage);
	}
}

UAmmoLoader* AAtomFirearm::GetAmmoLoader() const
{
	return AmmoLoader;
}

void AAtomFirearm::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (CartridgeEjectTemplate && GetNetMode() != ENetMode::NM_DedicatedServer)
	{
		CartridgeEjectComponent = UGameplayStatics::SpawnEmitterAttached(CartridgeEjectTemplate, GetMesh(), CartridgeAttachSocket, 
			FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, false);

		CartridgeEjectComponent->bAutoActivate = false;
		CartridgeEjectComponent->bAllowRecycling = true;

		if (CartridgeCollideSound)
		{
			CartridgeEjectComponent->OnParticleCollide.AddDynamic(this, &AAtomFirearm::OnEjectedCartridgeCollide);
		}		
	}	

	CartridgeMeshComponent->SetStaticMesh(CartridgeUnfiredMesh);

	if (AmmoLoader)
	{
		AmmoLoader->InitializeLoader();
	}	
}

bool AAtomFirearm::ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	WroteSomething |= Channel->ReplicateSubobject(AmmoLoader, *Bunch, *RepFlags);

	return WroteSomething;
}

void AAtomFirearm::OnEquipped()
{
	Super::OnEquipped();

	AmmoLoader->OnEquipped();
	bIsRecoilActive = false;
}

void AAtomFirearm::OnUnequipped()
{
	Super::OnUnequipped();

	AmmoLoader->OnUnequipped();
	bIsRecoilActive = false;
}

void AAtomFirearm::BeginPlay()
{
	Super::BeginPlay();

	AmmoLoader->BeginPlay();
}

void AAtomFirearm::Destroyed()
{
	AmmoLoader->FirearmDestroyed();

	Super::Destroyed();
}
