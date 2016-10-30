// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "EquippableState_ClipReload.h"
#include "FirearmClip.h"
#include "HeroFirearm.h"




void UEquippableState_ClipReload::OnEnteredState()
{
	Super::OnEnteredState();

	UPrimitiveComponent* ReloadTrigger = GetEquippable<AHeroFirearm>()->GetClipReloadTrigger();
	ReloadTrigger->OnComponentBeginOverlap.AddDynamic(this, &UEquippableState_ClipReload::OnClipEnteredReloadTrigger);
	ReloadTrigger->OnComponentEndOverlap.AddDynamic(this, &UEquippableState_ClipReload::OnClipExitedReloadTrigger);
}

void UEquippableState_ClipReload::OnExitedState()
{
	Super::OnExitedState();

	UPrimitiveComponent* ReloadTrigger = GetEquippable<AHeroFirearm>()->GetClipReloadTrigger();
	ReloadTrigger->OnComponentBeginOverlap.RemoveDynamic(this, &UEquippableState_ClipReload::OnClipEnteredReloadTrigger);
	ReloadTrigger->OnComponentEndOverlap.RemoveDynamic(this, &UEquippableState_ClipReload::OnClipExitedReloadTrigger);
}

void UEquippableState_ClipReload::OnClipEnteredReloadTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	check(Cast<AFirearmClip>(OtherActor) && "FirearmClip should be the only response to this trigger.");
	check(GetEquippable()->GetCurrentState() == this && "Overlap events should be unbound when not the active state.");

	AFirearmClip* OverlappingClip = static_cast<AFirearmClip*>(OtherActor);
	AHeroFirearm* Firearm = GetEquippable<AHeroFirearm>();

	if (OverlappingClip->GetHeroOwner() == Firearm->GetHeroOwner())
	{
		Firearm->AttachClip(OverlappingClip);
		Firearm->PopState(this);
	}
}

void UEquippableState_ClipReload::OnClipExitedReloadTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	check(Cast<AFirearmClip>(OtherActor) && "FirearmClip should be the only response to this trigger.");
	check(GetEquippable()->GetCurrentState() == this && "Overlap events should be unbound when not the active state.");
}
