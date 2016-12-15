// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomLoadoutTemplate.h"


UAtomLoadoutTemplate::UAtomLoadoutTemplate(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{

}

const TArray<FAtomLoadoutTemplateSlot>& UAtomLoadoutTemplate::GetLoadoutSlots() const
{
	return LoadoutSlots;
}

void UAtomLoadoutTemplate::PostLoad()
{
	Super::PostLoad();

	if (GetWorld() != nullptr && GetWorld()->WorldType == EWorldType::Game)
	{
		UE_LOG(AtomLog, Warning, TEXT("UHeroLoadoutTemplate should not be created. Use CDO instead."));
	}
}
