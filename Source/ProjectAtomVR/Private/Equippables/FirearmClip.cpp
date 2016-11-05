// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "FirearmClip.h"
#include "Components/MeshComponent.h"
#include "HeroFirearm.h"

namespace
{
	static constexpr float SmoothLoadSpeed = 30.f;
}

AFirearmClip::AFirearmClip(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UStaticMeshComponent>(AHeroEquippable::MeshComponentName))
{	
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	UMeshComponent* const MyMesh = GetMesh();
	MyMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	
	MyMesh->SetCanEverAffectNavigation(false);
	MyMesh->SetSimulatePhysics(false);
	MyMesh->bGenerateOverlapEvents = false;

	ClipLoadTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("ClipLoadTrigger"));
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
		SetCanReturnToLoadout(false);
		Unequip(EEquipType::Deferred);
	}

	PrimaryActorTick.SetTickFunctionEnable(true);

	SetActorEnableCollision(false);
	GetMesh()->SetSimulatePhysics(false);
	
	AttachToComponent(Firearm->GetMesh(), FAttachmentTransformRules::KeepWorldTransform, Firearm->GetMagazineAttachSocket());
}

void AFirearmClip::EjectFrom(class AHeroFirearm* Firearm)
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform); // Detach before replicating location to send world location

	if (HasAuthority())
	{		
		bReplicateMovement = true;
		ForceNetUpdate();
	}	

	bTearOff = true;
	ClipLoadTrigger->bGenerateOverlapEvents = false; // Disable trigger on eject, pending destroy

	GetMesh()->SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);
	SetActorEnableCollision(true);

	GetMesh()->SetSimulatePhysics(true);

	SetLifeSpan(10.f);
}

void AFirearmClip::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UMeshComponent* MyMesh = GetMesh();

	const FVector NewLocation = FMath::VInterpConstantTo(MyMesh->RelativeLocation, FVector::ZeroVector, DeltaSeconds, SmoothLoadSpeed);
	const FRotator NewRotation = FMath::RInterpConstantTo(MyMesh->RelativeRotation, FRotator::ZeroRotator, DeltaSeconds, SmoothLoadSpeed);
	MyMesh->SetRelativeLocationAndRotation(NewLocation, NewRotation);

	if (MyMesh->RelativeLocation.IsNearlyZero() && MyMesh->RelativeRotation.IsNearlyZero())
	{
		PrimaryActorTick.SetTickFunctionEnable(false);
	}
}
