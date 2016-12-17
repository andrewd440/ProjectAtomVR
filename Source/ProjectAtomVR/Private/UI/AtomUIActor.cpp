// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomUIActor.h"
#include "AtomUISystem.h"


// Sets default values
AAtomUIActor::AAtomUIActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;

}

class AAtomUISystem* AAtomUIActor::GetAtomUISystem() const
{
	check(GetOwner() == nullptr || Cast<AAtomUISystem>(GetOwner()));

	return static_cast<AAtomUISystem*>(GetOwner());
}
