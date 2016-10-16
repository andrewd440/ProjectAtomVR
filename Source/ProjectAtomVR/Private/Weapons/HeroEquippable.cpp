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
	AttachToComponent(OwningHero->GetHandMesh(Hand), FAttachmentTransformRules::SnapToTargetNotIncludingScale, HandAttachSocket);
}

bool AHeroEquippable::CanEquip(const EHand Hand) const
{
	return true;
}

void AHeroEquippable::Unequip()
{

}

void AHeroEquippable::OnRep_Owner()
{
	Super::OnRep_Owner();

	OwningHero = Cast<AHeroBase>(GetOwner());

	ensureMsgf(GetOwner() ? OwningHero != nullptr : true , TEXT("AHeroEquippable should only be owned by a AHeroBase."));
}

void AHeroEquippable::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);
	
	OwningHero = Cast<AHeroBase>(GetOwner());

	ensureMsgf(GetOwner() ? OwningHero != nullptr : true, TEXT("AHeroEquippable should only be owned by a AHeroBase."));
}

