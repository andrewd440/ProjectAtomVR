// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "FirearmHUDActor.h"
#include "FirearmWidget.h"
#include "AtomFirearm.h"

AFirearmHUDActor::AFirearmHUDActor()
	: Super()
{

}

void AFirearmHUDActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	for (auto* EquippableWidget : GetEquippableWidgets())
	{
		if (auto* FirearmWidget = Cast<UFirearmWidget>(EquippableWidget))
		{
			FirearmWidgets.Add(FirearmWidget);
		}
	}
}

void AFirearmHUDActor::BeginPlay()
{
	Super::BeginPlay();

	AAtomFirearm* Firearm = GetFirearm();
	Firearm->GetAmmoLoader()->OnAmmoCountChanged.BindUObject(this, &AFirearmHUDActor::OnAmmoCountChanged);
}

AAtomFirearm* AFirearmHUDActor::GetFirearm() const
{
	check(!GetEquippable() || Cast<AAtomFirearm>(GetEquippable())); // Might be null in editor

	return static_cast<AAtomFirearm*>(GetEquippable());
}

void AFirearmHUDActor::OnAmmoCountChanged()
{
	for (auto* FirearmWidget : FirearmWidgets)
	{
		FirearmWidget->OnAmmoCountChanged();
	}
}
