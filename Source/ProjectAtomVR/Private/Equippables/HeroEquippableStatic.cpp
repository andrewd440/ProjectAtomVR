// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "HeroEquippableStatic.h"




AHeroEquippableStatic::AHeroEquippableStatic(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UStaticMeshComponent>(AHeroEquippable::MeshComponentName))
{

}
