// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "FirearmWidget.h"
#include "FirearmHUDActor.h"

AFirearmHUDActor* UFirearmWidget::GetFirearmUI()
{
	check(!GetOwner() || Cast<AFirearmHUDActor>(GetOwner()) && "FirearmWidgets must only be used with FirearmUIActors"); // Might be null in editor
	return static_cast<AFirearmHUDActor*>(GetOwner());
}

class AAtomFirearm* UFirearmWidget::GetFirearm()
{
	return GetFirearmUI() ? GetFirearmUI()->GetFirearm() : nullptr;
}
