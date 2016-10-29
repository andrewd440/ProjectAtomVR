// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "FirearmClip.h"
#include "Components/MeshComponent.h"
#include "HeroFirearm.h"

AFirearmClip::AFirearmClip(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UStaticMeshComponent>(AHeroEquippable::MeshComponentName))
{	
	UMeshComponent* const MyMesh = GetMesh();
	MyMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

	MyMesh->SetCanEverAffectNavigation(false);
	MyMesh->SetSimulatePhysics(false);
	MyMesh->bGenerateOverlapEvents = false;

	SetActorEnableCollision(false);
}

int32 AFirearmClip::GetAmmoCount() const
{
	return AmmoCount;
}

void AFirearmClip::SetAmmoCount(int32 Count)
{
	AmmoCount = Count;
}

void AFirearmClip::OnClipAttached(class AHeroFirearm* Firearm)
{
	SetActorEnableCollision(false);

	GetMesh()->SetSimulatePhysics(false);
}

void AFirearmClip::OnClipEjected(class AHeroFirearm* Firearm)
{
	if (HasAuthority())
	{
		bReplicateMovement = true;
		ForceNetUpdate();

		TearOff();
	}

	GetMesh()->SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);
	SetActorEnableCollision(true);

	GetMesh()->SetSimulatePhysics(true);

	SetLifeSpan(10.f);
}
