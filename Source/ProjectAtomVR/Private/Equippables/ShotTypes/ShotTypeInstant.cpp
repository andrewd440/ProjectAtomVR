// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "ShotTypeInstant.h"
#include "AtomFirearm.h"
#include "Effects/AtomImpactEffect.h"

namespace
{
	static constexpr float MAX_SHOT_RANGE = 10000.f;
}

FShotData UShotTypeInstant::GetShotData() const
{
	const AAtomFirearm* const Firearm = GetFirearm();
	
	FShotData ShotData;
	ShotData.Start = Firearm->GetMuzzleLocation();
	ShotData.End = ShotData.Start + Firearm->GetMuzzleRotation().Vector() * MAX_SHOT_RANGE;

	ShotData.Seed = static_cast<uint8>(FMath::Rand() % 256);

	return ShotData;
}

void UShotTypeInstant::SimulateShot(const FShotData& ShotData)
{
	FMath::RandInit(ShotData.Seed);

	if (ShotCount > 1 || SpreadConeHalfAngleRad > 0.f)
	{
		const FVector ShotVector = (ShotData.End - ShotData.Start);
		const float ShotDistance = ShotVector.Size();

		for (int32 i = 0; i < ShotCount; ++i)
		{
			const FVector OffsetDirection = FMath::VRandCone(ShotVector, SpreadConeHalfAngleRad);

			const FHitResult Impact = WeaponTrace(ShotData.Start, OffsetDirection * ShotDistance);

			if (Impact.bBlockingHit)
				PlayImpactEffects(Impact);

			PlayTrailEffects(ShotData.Start, Impact.ImpactPoint);
		}
	}
	else
	{
		const FHitResult Impact = WeaponTrace(ShotData.Start, ShotData.End);

		if (Impact.bBlockingHit)
			PlayImpactEffects(Impact);

		PlayTrailEffects(ShotData.Start, Impact.ImpactPoint);
	}
}

void UShotTypeInstant::FireShot(const FShotData& ShotData)
{
	FMath::RandInit(ShotData.Seed);

	if (ShotCount > 1 || SpreadConeHalfAngleRad > 0.f)
	{
		const FVector ShotVector = (ShotData.End - ShotData.Start);
		const float ShotDistance = ShotVector.Size();

		for (int32 i = 0; i < ShotCount; ++i)
		{
			const FVector OffsetDirection = FMath::VRandCone(ShotVector, SpreadConeHalfAngleRad);

			const FHitResult Impact = WeaponTrace(ShotData.Start, OffsetDirection * ShotDistance);
			ProcessFiredShotImpact(Impact);
		}
	}
	else
	{
		const FHitResult Impact = WeaponTrace(ShotData.Start, ShotData.End);
		ProcessFiredShotImpact(Impact);
	}
}

void UShotTypeInstant::ProcessFiredShotImpact(const FHitResult& Impact)
{
	AAtomFirearm* const Firearm = GetFirearm();

	if (Impact.bBlockingHit && Impact.Actor.IsValid())
	{
		AActor& HitActor = *Impact.Actor;

		const float BaseDamage = Firearm->GetFirearmStats().Damage;
		const FPointDamageEvent DamageEvent{ BaseDamage, Impact, (Impact.TraceStart - Impact.TraceEnd).GetSafeNormal(), DamageType };

		HitActor.TakeDamage(BaseDamage, DamageEvent, Firearm->GetInstigatorController(), Firearm->GetHeroOwner());
	}

	// Play local effects
	if (Firearm->GetNetMode() != NM_DedicatedServer)
	{
		if (Impact.bBlockingHit)
			PlayImpactEffects(Impact);

		PlayTrailEffects(Impact.TraceStart, Impact.ImpactPoint);
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