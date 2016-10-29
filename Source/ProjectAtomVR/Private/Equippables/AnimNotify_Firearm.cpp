// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AnimNotify_Firearm.h"
#include "HeroFirearm.h"


void UAnimNotify_Firearm::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (AHeroFirearm* Firearm = Cast<AHeroFirearm>(MeshComp->GetOwner()))
	{
		Firearm->OnFirearmNotify(Type);
	}
}
