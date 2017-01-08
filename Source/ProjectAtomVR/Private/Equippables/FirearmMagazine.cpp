// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "FirearmMagazine.h"
#include "Components/MeshComponent.h"
#include "AtomFirearm.h"


AFirearmMagazine::AFirearmMagazine(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UStaticMeshComponent>(AAtomEquippable::MeshComponentName))
{	
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	UStaticMeshComponent* MyMesh = GetMesh<UStaticMeshComponent>();
	MyMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	
	MyMesh->SetCanEverAffectNavigation(false);
	MyMesh->SetSimulatePhysics(false);
	MyMesh->bGenerateOverlapEvents = false;

	LoadTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("LoadTrigger"));
	LoadTrigger->SetIsReplicated(false);
	LoadTrigger->SetupAttachment(MyMesh);
	LoadTrigger->bGenerateOverlapEvents = true;
	LoadTrigger->SetCollisionObjectType(AtomCollisionChannels::ClipLoadTrigger);
	LoadTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	LoadTrigger->SetCollisionResponseToChannel(AtomCollisionChannels::FirearmReloadTrigger, ECollisionResponse::ECR_Overlap);
}
