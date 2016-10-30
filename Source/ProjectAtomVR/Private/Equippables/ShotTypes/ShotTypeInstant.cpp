// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "ShotTypeInstant.h"
#include "HeroFirearm.h"
#include "Effects/AtomImpactEffect.h"

namespace
{
	static constexpr float MAX_SHOT_RANGE = 10000.f;
}

FShotData UShotTypeInstant::GetShotData() const
{
	const AHeroFirearm* const Firearm = GetFirearm();
	
	FShotData ShotData;
	ShotData.Start = Firearm->GetMuzzleLocation();
	ShotData.End = ShotData.Start + Firearm->GetMuzzleRotation().Vector() * MAX_SHOT_RANGE;

	return ShotData;
}

void UShotTypeInstant::SimulateShot(const FShotData& ShotData)
{
	const FHitResult Impact = WeaponTrace(ShotData.Start, ShotData.End);

	if (Impact.bBlockingHit)
		PlayImpactEffects(Impact);

	PlayTrailEffects(ShotData.Start, Impact.ImpactPoint);
}

void UShotTypeInstant::FireShot(const FShotData& ShotData)
{
	const FHitResult Impact = WeaponTrace(ShotData.Start, ShotData.End);

	AHeroFirearm* const Firearm = GetFirearm();

	if (Impact.bBlockingHit && Impact.Actor.IsValid())
	{
		AActor& HitActor = *Impact.Actor;

		const float BaseDamage = Firearm->GetFirearmStats().Damage;
		const FPointDamageEvent DamageEvent{ BaseDamage, Impact, (ShotData.Start - ShotData.End).GetSafeNormal(), DamageType };

		HitActor.TakeDamage(BaseDamage, DamageEvent, Firearm->GetInstigatorController(), Firearm->GetHeroOwner());
	}

	// Play local effects
	if (Firearm->GetNetMode() != NM_DedicatedServer)
	{
		if (Impact.bBlockingHit)
			PlayImpactEffects(Impact);

		PlayTrailEffects(ShotData.Start, Impact.ImpactPoint);
	}
}

void UShotTypeInstant::PlayTrailEffects(const FVector& Start, const FVector& End) const
{
	if (TrailFX)
	{
		const FVector DirectionVector = (End - Start);

		UParticleSystemComponent* TrailComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TrailFX, Start, DirectionVector.GetUnsafeNormal().Rotation());
		TrailComponent->SetFloatParameter(TEXT("Distance"), DirectionVector.Size());
		
	}
}

void UShotTypeInstant::PlayImpactEffects(const FHitResult& Hit) const
{
	if (ImpactEffect)
	{
		const UAtomImpactEffect* const EffectObject = ImpactEffect->GetDefaultObject<UAtomImpactEffect>();
		EffectObject->SpawnEffect(GetWorld(), Hit);
	}
}

FHitResult UShotTypeInstant::WeaponTrace(const FVector& Start, const FVector& End) const
{
	FHitResult Impact;
	FCollisionQueryParams QueryParams{ NAME_None, false };
	QueryParams.bReturnPhysicalMaterial = true;

	if (ShotRadius <= 0)
	{
		if (!GetWorld()->LineTraceSingleByChannel(Impact, Start, End, CollisionChannelAliases::InstantShot, QueryParams))
		{
			Impact.ImpactPoint = Impact.TraceEnd;
		}
	}
	else
	{
		if (!GetWorld()->SweepSingleByChannel(Impact, Start, End, FQuat::Identity, CollisionChannelAliases::InstantShot, FCollisionShape::MakeSphere(ShotRadius), QueryParams))
		{
			Impact.ImpactPoint = Impact.TraceEnd;
		}
	}

	return Impact;
}