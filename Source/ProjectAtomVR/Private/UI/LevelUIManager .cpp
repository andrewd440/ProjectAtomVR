// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "LevelUIManager.h"


ALevelUIManager::ALevelUIManager()
{

}

void ALevelUIManager::SpawnLevelUI(class AAtomUISystem* OwningSystem)
{
	TInlineComponentArray<ULevelUIComponent*> UIComponents;
	GetComponents(UIComponents);

	for (ULevelUIComponent* UIComponent : UIComponents)
	{
		if (UIComponent->GetSpawnType() == ELevelUISpawnType::WithLevel)
		{
			UIComponent->SpawnUIActor(OwningSystem);
		}
	}
}