// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "EquippableState_ClipReload.h"
#include "FirearmClip.h"
#include "HeroFirearm.h"

namespace
{
	static constexpr float ClipAttachRotationErrorDegrees = 15.f;
	static constexpr float ClipAttachRotationErrorRadians = ClipAttachRotationErrorDegrees * (PI / 180.f);
}

UEquippableState_ClipReload::UEquippableState_ClipReload(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	bRequiresEquippedClip = true;
}

void UEquippableState_ClipReload::OnEnteredState()
{
	Super::OnEnteredState();

	if (GetEquippable()->GetHeroOwner()->IsLocallyControlled())
	{
		UPrimitiveComponent* ReloadTrigger = GetEquippable<AHeroFirearm>()->GetMagazineReloadTrigger();
		ReloadTrigger->OnComponentBeginOverlap.AddDynamic(this, &UEquippableState_ClipReload::OnClipEnteredReloadTrigger);
	}
}

void UEquippableState_ClipReload::OnExitedState()
{
	Super::OnExitedState();

	if (GetEquippable()->GetHeroOwner()->IsLocallyControlled())
	{
		UPrimitiveComponent* ReloadTrigger = GetEquippable<AHeroFirearm>()->GetMagazineReloadTrigger();
		ReloadTrigger->OnComponentBeginOverlap.RemoveDynamic(this, &UEquippableState_ClipReload::OnClipEnteredReloadTrigger);
	}
}

void UEquippableState_ClipReload::OnClipEnteredReloadTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	check(Cast<AFirearmClip>(OtherActor) && "FirearmClip should be the only response to this trigger.");
	check(GetEquippable()->GetCurrentState() == this && "Overlap events should be unbound when not the active state.");

	AFirearmClip* OverlappingClip = static_cast<AFirearmClip*>(OtherActor);
	AHeroFirearm* Firearm = GetEquippable<AHeroFirearm>();

	if (OverlappingClip->GetHeroOwner() == Firearm->GetHeroOwner() &&
		(!bRequiresEquippedClip || OverlappingClip->IsEquipped()) &&
		OverlappingClip->IsA(Firearm->GetMagazineClass()))
	{
		FQuat AttachRotation = Firearm->GetMesh<UMeshComponent>()->GetSocketQuaternion(Firearm->GetMagazineAttachSocket());
		FQuat ClipRotation = OverlappingClip->GetActorQuat();

		if (AttachRotation.AngularDistance(ClipRotation) <= ClipAttachRotationErrorRadians)
		{
			Firearm->AttachMagazine(OverlappingClip);
		}
	}
}