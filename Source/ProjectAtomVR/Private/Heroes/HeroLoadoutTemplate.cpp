// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "HeroLoadoutTemplate.h"


UHeroLoadoutTemplate::UHeroLoadoutTemplate(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{

}

const TArray<FHeroLoadoutTemplateSlot>& UHeroLoadoutTemplate::GetLoadoutSlots() const
{
	return LoadoutSlots;
}

void UHeroLoadoutTemplate::PostLoad()
{
	Super::PostLoad();

	if (GetWorld() != nullptr && GetWorld()->WorldType == EWorldType::Game)
	{
		UE_LOG(AtomLog, Warning, TEXT("UHeroLoadoutTemplate should not be created. Use CDO instead."));
	}
}
