// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomControlPoint.h"




AAtomControlPoint::AAtomControlPoint()
{
	CaptureBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("CaptureBounds"));
	CaptureBounds->SetBoxExtent(FVector{ 250, 250, 100 }, false);
	CaptureBounds->SetCollisionProfileName(AtomCollisionProfiles::ObjectiveTrigger);
	RootComponent = CaptureBounds;

	OutlineMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OutlineMesh"));
	OutlineMesh->bGenerateOverlapEvents = false;
	OutlineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OutlineMesh->SetupAttachment(CaptureBounds);

	InnerSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InnerSphere"));
	InnerSphere->bGenerateOverlapEvents = false;
	InnerSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InnerSphere->SetupAttachment(CaptureBounds);

	OuterSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OuterSphere"));
	OuterSphere->bGenerateOverlapEvents = false;
	OuterSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OuterSphere->SetupAttachment(InnerSphere);

	SetActorHiddenInGame(true);
}

void AAtomControlPoint::InitializeObjective()
{
	Super::InitializeObjective();

	SetActorHiddenInGame(false);
}
