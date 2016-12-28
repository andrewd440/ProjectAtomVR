// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "LevelUIManager.generated.h"

UCLASS()
class PROJECTATOMVR_API ALevelUIManager : public AActor
{
	GENERATED_BODY()
	
public:
	ALevelUIManager();	

	void SpawnLevelUI(class AAtomUISystem* OwningSystem);
};
