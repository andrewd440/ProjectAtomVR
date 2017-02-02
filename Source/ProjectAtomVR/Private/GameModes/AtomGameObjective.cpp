// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomGameObjective.h"


// Sets default values
AAtomGameObjective::AAtomGameObjective()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	bNetLoadOnClient = true;
	bAlwaysRelevant = true;
	bCanBeDamaged = false;
}

void AAtomGameObjective::InitializeObjective()
{

}
