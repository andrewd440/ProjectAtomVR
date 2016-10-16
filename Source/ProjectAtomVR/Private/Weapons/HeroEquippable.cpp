// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "HeroEquippable.h"


// Sets default values
AHeroEquippable::AHeroEquippable()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));	
	Mesh->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered;
	RootComponent = Mesh;
}

// Called when the game starts or when spawned
void AHeroEquippable::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AHeroEquippable::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

void AHeroEquippable::Equip(const EHand Hand)
{
	AttachToComponent(HeroOwner->GetHandMesh(Hand), FAttachmentTransformRules::SnapToTargetNotIncludingScale, HandAttachSocket);
}

bool AHeroEquippable::CanEquip(const EHand Hand) const
{
	return !IsEquipped();
}

void AHeroEquippable::Unequip()
{
	if (StorageAttachComponent)
	{
		AttachToComponent(StorageAttachComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, StorageAttachSocket);
	}
	else
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}
}

void AHeroEquippable::SetLoadoutAttachment(USceneComponent* AttachComponent, FName AttachSocket)
{
	StorageAttachComponent = AttachComponent;
	StorageAttachSocket = AttachSocket;
}

bool AHeroEquippable::IsEquipped() const
{
	return HeroOwner->GetEquippable<EHand::Left>() == this || 
		   HeroOwner->GetEquippable<EHand::Right>() == this;
}

void AHeroEquippable::OnRep_Owner()
{
	Super::OnRep_Owner();

	HeroOwner = Cast<AHeroBase>(GetOwner());

	ensureMsgf(GetOwner() ? HeroOwner != nullptr : true , TEXT("AHeroEquippable should only be owned by a AHeroBase."));
}

void AHeroEquippable::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);
	
	HeroOwner = Cast<AHeroBase>(GetOwner());

	ensureMsgf(GetOwner() ? HeroOwner != nullptr : true, TEXT("AHeroEquippable should only be owned by a AHeroBase."));
}
