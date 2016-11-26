// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "FirearmUIActor.h"
#include "../UMG/Public/Components/WidgetComponent.h"
#include "HeroFirearm.h"
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

	AHeroFirearm* Firearm = GetFirearm();
	Firearm->OnAmmoCountChanged.BindUObject(this, &AFirearmUIActor::OnAmmoCountChanged);
}

AHeroFirearm* AFirearmUIActor::GetFirearm() const
{
	check(Cast<AHeroFirearm>(GetOwner()));

	return static_cast<AHeroFirearm*>(GetOwner());
}

void AFirearmUIActor::OnAmmoCountChanged()
{
	for (auto* FirearmWidget : FirearmWidgets)
	{
		FirearmWidget->OnAmmoCountChanged();
	}
}
