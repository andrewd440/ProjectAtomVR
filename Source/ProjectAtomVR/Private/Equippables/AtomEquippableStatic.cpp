// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomEquippableStatic.h"




AAtomEquippableStatic::AAtomEquippableStatic(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UStaticMeshComponent>(AAtomEquippable::MeshComponentName))
{

}
