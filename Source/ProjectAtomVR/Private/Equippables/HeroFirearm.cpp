// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "HeroFirearm.h"
#include "HeroBase.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"

AHeroFirearm::AHeroFirearm(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	Stats.Damage = 20;
	Stats.FireRate = 0.1f;
	Stats.MaxAmmo = 160;
	Stats.ClipSize = 30;
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

		PlayFiringEffects();
		ConsumeAmmo();
	}
	else if(Role == ENetRole::ROLE_SimulatedProxy)
	{
		ShotType->SimulateShot(ShotData);
		PlayFiringEffects();
		ConsumeAmmo();
	}
}

void AHeroFirearm::ServerFireShot_Implementation(FShotData ShotData)
{
	ConsumeAmmo();
	ShotType->FireShot(ShotData);

	if (GetNetMode() != ENetMode::NM_DedicatedServer)
	{
		PlayFiringEffects();
	}
}

bool AHeroFirearm::ServerFireShot_Validate(FShotData ShotData)
{
	return true;
}

void AHeroFirearm::PlayFiringEffects()
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
}

void AHeroFirearm::StopFiringEffects()
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
}

void AHeroFirearm::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	RemainingAmmo = Stats.MaxAmmo - Stats.ClipSize;
	RemainingClip = Stats.ClipSize;
}