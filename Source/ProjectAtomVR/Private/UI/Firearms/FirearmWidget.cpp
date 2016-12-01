// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "FirearmWidget.h"
#include "FirearmUIActor.h"

AFirearmUIActor* UFirearmWidget::GetFirearmUI()
{
	check(!GetOwner() || Cast<AFirearmUIActor>(GetOwner()) && "FirearmWidgets must only be used with FirearmUIActors"); // Might be null in editor
	return static_cast<AFirearmUIActor*>(GetOwner());
}

class AHeroFirearm* UFirearmWidget::GetFirearm()
{
	check(GetFirearmUI());
	return GetFirearmUI()->GetFirearm();
}
