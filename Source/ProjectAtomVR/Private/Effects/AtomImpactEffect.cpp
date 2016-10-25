// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomImpactEffect.h"

#include "PhysicalMaterials/PhysicalMaterial.h"

DEFINE_LOG_CATEGORY_STATIC(LogImpactEffect, Log, All);

void UAtomImpactEffect::SpawnEffect(UWorld* World, const FHitResult& Hit) const
{
	if (!World)
	{
		UE_LOG(LogImpactEffect, Warning, TEXT("AtomImpactEffect::SpawnEffect World parameter is invalid."));
	}
	else
	{
		const FMaterialEffect& Effect = GetEffect(Hit.PhysMaterial);
		const FRotator Rotation = Hit.ImpactNormal.Rotation();

		if (Effect.Particles)
		{
			// Reflect the particle system on the surface
			const FVector HitDirection = Hit.ImpactPoint - Hit.TraceStart;
			const FVector ParticleDirection = HitDirection.MirrorByVector(Hit.ImpactNormal);
			UGameplayStatics::SpawnEmitterAtLocation(World, Effect.Particles, Hit.ImpactPoint, ParticleDirection.Rotation());
		}

		if (Effect.Sound)
		{
			UGameplayStatics::SpawnSoundAtLocation(World, Effect.Sound, Hit.ImpactPoint, Rotation);
		}

		if (DecalInfo.Material)
		{
			const FRotator DecalRotation = Rotation;

			// #bstodo Apply random rotation to decal
			UGameplayStatics::SpawnDecalAttached(DecalInfo.Material, DecalInfo.DecalSize, Hit.Component.Get(), Hit.BoneName,
				Hit.ImpactPoint, DecalRotation, EAttachLocation::KeepWorldPosition, DecalInfo.LifeSpan);
		}
	}
}

void UAtomImpactEffect::BeginDestroy()
{
	Super::BeginDestroy();

	if (GetWorld() != nullptr && GetWorld()->WorldType == EWorldType::Game)
	{
		if (GAreScreenMessagesEnabled)
		{
			GEngine->AddOnScreenDebugMessage((uint64)-1, 3.0f, FColor::Red,
				*FString::Printf(TEXT("AtomImpactEffect should not be spawned. Use SpawnEffect on default class. %s"), *GetName()));
		}

		UE_LOG(LogImpactEffect, Warning, TEXT("AtomImpactEffect should not be instanced. Use DefaultObject instead."));
	}
}

const FMaterialEffect& UAtomImpactEffect::GetEffect(TWeakObjectPtr<UPhysicalMaterial> Material) const
{
	if (Material.IsValid())
	{
		const int32 Index = static_cast<int32>(Material->SurfaceType.GetValue());
		return SurfaceEffects[Index];
	}
	else
	{
		return SurfaceEffects[0];
	}
}
