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

	ClipLoadTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("ClipLoadTrigger"));
	ClipLoadTrigger->SetIsReplicated(false);
	ClipLoadTrigger->SetupAttachment(MyMesh);
	ClipLoadTrigger->bGenerateOverlapEvents = true;
	ClipLoadTrigger->SetCollisionObjectType(CollisionChannelAliases::ClipLoadTrigger);
	ClipLoadTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	ClipLoadTrigger->SetCollisionResponseToChannel(CollisionChannelAliases::FirearmReloadTrigger, ECollisionResponse::ECR_Overlap);
}

void AFirearmClip::LoadInto(class AHeroFirearm* Firearm)
{
	if (EquipStatus.bIsEquipped)
	{
		bReturnToStorage = false;
		Unequip();
	}

	SetActorEnableCollision(false);
	GetMesh()->SetSimulatePhysics(false);
	
	AttachToComponent(Firearm->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, Firearm->GetClipAttachSocket());
}

void AFirearmClip::EjectFrom(class AHeroFirearm* Firearm)
{
	if (HasAuthority())
	{
		bReplicateMovement = true;
		ForceNetUpdate();

		TearOff();
	}

	ClipLoadTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Disable trigger on eject, pending destroy

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	GetMesh()->SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);
	SetActorEnableCollision(true);

	GetMesh()->SetSimulatePhysics(true);

	SetLifeSpan(10.f);
}
