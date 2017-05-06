// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomHUDActor.h"
#include "WidgetComponent.h"
#include "UserWidget.h"
#include "WidgetTree.h"
#include "VRHUD.h"


// Sets default values
AAtomHUDActor::AAtomHUDActor()
{
	bNetLoadOnClient = false;
}

class AVRHUD* AAtomHUDActor::GetHUD() const
{
	check(CastChecked<AVRHUD>(GetOwner(), ECastCheckedType::NullAllowed));

	return Cast<AVRHUD>(GetOwner());
}