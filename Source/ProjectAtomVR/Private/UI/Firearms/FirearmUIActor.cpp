// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "FirearmUIActor.h"
#include "../UMG/Public/Components/WidgetComponent.h"
#include "AtomFirearm.h"
#include "Firearms/FirearmWidget.h"

AFirearmUIActor::AFirearmUIActor()
	: Super()
{

}

void AFirearmUIActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	for (auto* EquippableWidget : GetWidgets())
	{
		if (auto* FirearmWidget = Cast<UFirearmWidget>(EquippableWidget))
		{
			FirearmWidgets.Add(FirearmWidget);
		}
	}
}

void AFirearmUIActor::BeginPlay()
{
	Super::BeginPlay();

	AAtomFirearm* Firearm = GetFirearm();
	Firearm->GetAmmoLoader()->OnAmmoCountChanged.BindUObject(this, &AFirearmUIActor::OnAmmoCountChanged);
}

AAtomFirearm* AFirearmUIActor::GetFirearm() const
{
	check(!GetOwner() || Cast<AAtomFirearm>(GetOwner())); // Might be null in editor

	return static_cast<AAtomFirearm*>(GetOwner());
}

void AFirearmUIActor::OnAmmoCountChanged()
{
	for (auto* FirearmWidget : FirearmWidgets)
	{
		FirearmWidget->OnAmmoCountChanged();
	}
}
