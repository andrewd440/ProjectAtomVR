// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "LevelActorManager.generated.h"

UCLASS()
class PROJECTATOMVR_API ALevelActorManager : public AActor
{
	GENERATED_BODY()
	
public:
	ALevelActorManager();	

	void SpawnLevelActors();
};
