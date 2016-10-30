// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "HeroFirearm.h"
#include "HeroBase.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimInstance.h"
#include "AnimNotify_Firearm.h"
#include "FirearmClip.h"

DEFINE_LOG_CATEGORY_STATIC(LogFirearm, Log, All);

AHeroFirearm::AHeroFirearm(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<USkeletalMeshComponent>(AHeroEquippable::MeshComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	Stats.Damage = 20;
	Stats.FireRate = 0.1f;
	Stats.MaxAmmo = 160;
	Stats.ClipSize = 30;

	bNeedsBoltPull = false;

	GetSkeletalMesh()->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered;

	ClipReloadTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("ClipReloadTrigger"));
	ClipReloadTrigger->SetIsReplicated(false);
	ClipReloadTrigger->bGenerateOverlapEvents = true;
	ClipReloadTrigger->SetCollisionObjectType(CollisionChannelAliases::FirearmReloadTrigger);
	ClipReloadTrigger->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	ClipReloadTrigger->SetCollisionResponseToChannel(CollisionChannelAliases::ClipLoadTrigger, ECollisionResponse::ECR_Overlap);
}


void AHeroFirearm::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

FVector AHeroFirearm::GetMuzzleLocation() const
{
	return GetMesh()->GetSocketLocation(MuzzleSocket);
}

FQuat AHeroFirearm::GetMuzzleRotation() const
{
	return GetMesh()->GetSocketQuaternion(MuzzleSocket);
}

void AHeroFirearm::AttachClip(AFirearmClip* Clip)
{
	check(Clip);
	ensure(CurrentClip == nullptr);

	if (!HasAuthority() && GetHeroOwner()->IsLocallyControlled())
	{
		ServerAttachClip(Clip);
	}

	CurrentClip = Clip;
	
	Clip->LoadInto(this);
}

void AHeroFirearm::EjectClip()
{
	if (GetHeroOwner()->IsLocallyControlled() && !HasAuthority())
	{
		ServerEjectClip();
	}

	if (CurrentClip)
	{		
		CurrentClip->EjectFrom(this);
	}

	CurrentClip = nullptr;
}

void AHeroFirearm::ServerAttachClip_Implementation(class AFirearmClip* Clip)
{
	AttachClip(Clip);
}

bool AHeroFirearm::ServerAttachClip_Validate(class AFirearmClip* Clip)
{
	return true;
}

void AHeroFirearm::ServerEjectClip_Implementation()
{
	EjectClip();
}

bool AHeroFirearm::ServerEjectClip_Validate()
{
	return true;
}

void AHeroFirearm::ConsumeAmmo()
{
	--RemainingClip;
}

void AHeroFirearm::FireShot()
{
	check(ShotType);

	const FShotData ShotData = ShotType->GetShotData();

	// Since the firing state will be call FireShot while active, it will be
	// call by Autonomous, Authority, and Simulated connections. We will allow simulated
	// proxies to simulate fire. Autonomous will simulate and call ServerFire. Authority
	// will only fire here if it is locally controlled. Otherwise, it will wait on the controlling
	// client to sent a fire event through ServerFireShot.
	if (GetHeroOwner()->IsLocallyControlled())
	{
		if (HasAuthority())
		{			
			ShotType->FireShot(ShotData);
		}
		else
		{
			ShotType->SimulateShot(ShotData);
			ServerFireShot(ShotData);
		}

		PlaySingleShotSequence();
		ConsumeAmmo();
	}
	else if(Role == ENetRole::ROLE_SimulatedProxy)
	{
		ShotType->SimulateShot(ShotData);
		PlaySingleShotSequence();
		ConsumeAmmo();
	}
}

void AHeroFirearm::StartFiringSequence()
{
	if (TriggerPullMontage)
	{
		GetSkeletalMesh()->GetAnimInstance()->Montage_Play(TriggerPullMontage);
	}
}

void AHeroFirearm::ServerFireShot_Implementation(FShotData ShotData)
{
	ConsumeAmmo();
	ShotType->FireShot(ShotData);

	if (GetNetMode() != ENetMode::NM_DedicatedServer)
	{
		PlaySingleShotSequence();
	}
}

bool AHeroFirearm::ServerFireShot_Validate(FShotData ShotData)
{
	return true;
}

void AHeroFirearm::PlaySingleShotSequence()
{
	// Activate if not active or not looping
	if (MuzzleFX != nullptr && (MuzzleFXComponent == nullptr || !MuzzleFX->IsLooping()))
	{
		MuzzleFXComponent = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, GetMesh(), MuzzleSocket);
		MuzzleFXComponent->ActivateSystem();
	}

	if (FireSound && (!FireSound->IsLooping() || !FireSoundComponent))
	{
		FireSoundComponent = UGameplayStatics::SpawnSoundAttached(FireSound, GetMesh(), MuzzleSocket);
		FireSoundComponent->Play();
	}

	if (BoltCarrierMontage)
	{
		GetSkeletalMesh()->GetAnimInstance()->Montage_Play(BoltCarrierMontage);
	}
}

void AHeroFirearm::OnRep_CurrentClip()
{
	if (CurrentClip == nullptr)
	{
		CurrentClip = RemoteConnectionClip;
		EjectClip();
	}
	else
	{
		// Simulate attaching a new clip
		AFirearmClip* NewClip = CurrentClip;
		CurrentClip = nullptr;
		AttachClip(NewClip);
	}

	RemoteConnectionClip = CurrentClip;
}

void AHeroFirearm::OnRep_DefaultClip()
{
	if (RemoteConnectionClip)
	{
		CurrentClip = RemoteConnectionClip;
		
		CurrentClip->LoadInto(this);
	}
}

void AHeroFirearm::BeginPlay()
{
	Super::BeginPlay();
}

void AHeroFirearm::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AHeroFirearm, CurrentClip, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AHeroFirearm, RemoteConnectionClip, COND_OwnerOnly);
}

void AHeroFirearm::StopFiringSequence()
{
	if (MuzzleFXComponent && MuzzleFXComponent->IsActive() && MuzzleFXComponent->Template->IsLooping())
	{
		MuzzleFXComponent->DeactivateSystem();
		MuzzleFXComponent = nullptr;
	}

	if (FireSoundComponent && FireSound->IsLooping())
	{
		FireSoundComponent->FadeOut(0.1f, 0.0f);
		FireSoundComponent = nullptr;
	}

	if (EndFireSound)
	{
		UGameplayStatics::SpawnSoundAttached(EndFireSound, GetMesh(), MuzzleSocket);
	}

	if (TriggerPullMontage)
	{
		GetSkeletalMesh()->GetAnimInstance()->Montage_Stop(.1f, TriggerPullMontage);
	}
}

void AHeroFirearm::OnFirearmNotify(EFirearmNotify Type)
{
	if (Type == EFirearmNotify::ShellEject)
	{
		if (ShellEjectComponent)
		{
			ShellEjectComponent->Activate(true);
		}		
	}
}

void AHeroFirearm::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	RemainingAmmo = Stats.MaxAmmo - Stats.ClipSize;
	RemainingClip = Stats.ClipSize;

	ClipReloadTrigger->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, ClipAttachSocket);

	if (ShellEjectTemplate)
	{
		ShellEjectComponent = UGameplayStatics::SpawnEmitterAttached(ShellEjectTemplate, GetMesh(), ShellEjectSocket, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, false);
		ShellEjectComponent->bAutoActivate = false;
	}

	if (HasAuthority() && ClipType)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetHeroOwner();
		RemoteConnectionClip = GetWorld()->SpawnActor<AFirearmClip>(ClipType, FTransform::Identity, SpawnParams);
		AttachClip(RemoteConnectionClip);
	}
}