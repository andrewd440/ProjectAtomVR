// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "AtomGameObjective.generated.h"

UCLASS()
class PROJECTATOMVR_API AAtomGameObjective : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAtomGameObjective();

	virtual void InitializeObjective();
};
