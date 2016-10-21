// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "HeroWeapon.h"
#include "HeroBase.h"

AHeroWeapon::AHeroWeapon(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}


void AHeroWeapon::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

void AHeroWeapon::Equip(class AHeroBase* EquippingHero)
{
	//Mesh->AttachToComponent(EquippingHero->GetHandMesh<EHandType::Dominate>(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, EquippingHero->GetEquipWeaponSocket());
}

