// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "LevelActorManager.h"


ALevelActorManager::ALevelActorManager()
{

}

void ALevelActorManager::SpawnLevelActors()
{
	TInlineComponentArray<ULevelActorComponent*> LevelActorComponents;
	GetComponents(LevelActorComponents);

	for (ULevelActorComponent* ActorComponent : LevelActorComponents)
	{
		if (ActorComponent->GetSpawnType() == ELevelUISpawnType::WithLevel)
		{
			ActorComponent->SpawnActor();
		}
	}
}