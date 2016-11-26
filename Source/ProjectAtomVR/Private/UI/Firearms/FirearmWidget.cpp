// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "FirearmWidget.h"
#include "UI/FirearmUIActor.h"

AFirearmUIActor* UFirearmWidget::GetFirearmUI()
{
	check(Cast<AFirearmUIActor>(GetOwner()) && "FirearmWidgets must only be used with FirearmUIActors");
	return static_cast<AFirearmUIActor*>(GetOwner());
}
