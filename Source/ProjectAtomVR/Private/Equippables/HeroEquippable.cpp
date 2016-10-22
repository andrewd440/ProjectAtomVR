// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "HeroEquippable.h"
#include "Animation/AnimSequence.h"
#include "EquippableState.h"
#include "EquippableStateInactive.h"
#include "EquippableStateActive.h"


// Sets default values
AHeroEquippable::AHeroEquippable(const FObjectInitializer& ObjectInitializer/* = FObjectInitializer::Get()*/)
{
	bReplicates = true;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));	
	Mesh->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered;
	RootComponent = Mesh;
}

void AHeroEquippable::Equip(const EHand Hand)
{
	ensureMsgf(StateStack.Top() == InactiveState, TEXT("StateStack should only have the InactiveState when equipping."));

	EquipStatus.EquippedHand = Hand;
	EquipStatus.bIsEquipped = true;

	if(!HeroOwner->HasAuthority())
	{
		ServerEquip(Hand);
	}
	else
	{
		// Make sure the equip update is sent to clients
		EquipStatus.ForceReplication();
	}

	StateStack.Push(ActiveState);
	ActiveState->OnEnteredState();
}

bool AHeroEquippable::CanEquip(const EHand Hand) const
{
	return !IsEquipped() && GetWorld()->GetTimeSeconds() > UnequipTimeStamp + ReuseDelay;
}

void AHeroEquippable::Unequip()
{
	ensure(EquipStatus.bIsEquipped);
	EquipStatus.bIsEquipped = false;

	if (!HeroOwner->HasAuthority())
	{
		ServerUnequip();
	}
	else
	{
		// Make sure the equip update is sent to clients
		EquipStatus.ForceReplication();
	}

	while (StateStack.Num() > 1)
	{
		StateStack.Top()->OnUnequip();
		StateStack.Pop(false);
	}

	// Notify inactive state of entered event
	ensure(StateStack.Top() == InactiveState);
	StateStack.Top()->OnReturnedState();
}

void AHeroEquippable::SetStorageAttachment(USceneComponent* AttachComponent, FName AttachSocket)
{
	StorageAttachComponent = AttachComponent;
	StorageAttachSocket = AttachSocket;
}

bool AHeroEquippable::IsEquipped() const
{
	return StateStack.Top() != InactiveState;
}

void AHeroEquippable::PushActiveState()
{
	PushState(ActiveState);
}

void AHeroEquippable::PushState(UEquippableState* InPushState)
{
	ensure(StateStack.Find(InPushState) == INDEX_NONE);
	check(InPushState);

	StateStack.Push(InPushState);
	InPushState->OnEnteredState();
}

void AHeroEquippable::PopState(UEquippableState* InPopState)
{
	ensure(StateStack.Top() == InPopState);
	ensure(StateStack.Num() > 0);
	check(InPopState);

	StateStack.Pop(false); // no need to shrink, it'll probably be added again

	StateStack.Top()->OnReturnedState();
}

void AHeroEquippable::OnRep_EquipStatus()
{
	if (EquipStatus.bIsEquipped)
	{
		ensureMsgf(StateStack.Top() == InactiveState, TEXT("StateStack should only have the InactiveState when equipping."));
		StateStack.Push(ActiveState);
		ActiveState->OnEnteredState();
	}
	else
	{
		while (StateStack.Num() > 1)
		{
			StateStack.Top()->OnUnequip();
			StateStack.Pop(false);
		}

		// Notify inactive state of entered event
		ensure(StateStack.Top() == InactiveState);
		StateStack.Top()->OnReturnedState();
	}
}

void AHeroEquippable::OnRep_Owner()
{
	Super::OnRep_Owner();

	HeroOwner = Cast<AHeroBase>(GetOwner());

	ensureMsgf(GetOwner() ? HeroOwner != nullptr : true , TEXT("AHeroEquippable should only be owned by a AHeroBase."));
}

void AHeroEquippable::OnEquipped()
{
	USkeletalMeshComponent* const AttachHand = HeroOwner->GetHandMesh(EquipStatus.EquippedHand);

	// Only allow attachments on server or locally controlled. Remotes will get replicated attachments.
	if (HeroOwner->HasAuthority() || HeroOwner->IsLocallyControlled())
	{
		// Set return storage before attaching to hero
		SetStorageAttachment(Mesh->GetAttachParent(), Mesh->GetAttachSocketName());

		AttachToComponent(AttachHand, FAttachmentTransformRules::SnapToTargetNotIncludingScale, HandAttachSocket);
	}

	UAnimSequence* const HandAnim = (EquipStatus.EquippedHand == EHand::Right) ? AnimHandEquip.Right : AnimHandEquip.Left;
	if (HandAnim)
	{
		AttachHand->PlayAnimation(HandAnim, true);
	}

	HeroOwner->OnEquipped(this, EquipStatus.EquippedHand);
}

void AHeroEquippable::OnUnequipped()
{
	UnequipTimeStamp = GetWorld()->GetTimeSeconds();

	// Only allow attachments on server or locally controlled. Remotes will get replicated attachments.
	if (HeroOwner->HasAuthority() || HeroOwner->IsLocallyControlled())
	{
		if (StorageAttachComponent)
		{
			AttachToComponent(StorageAttachComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, StorageAttachSocket);
		}
		else
		{
			DetachFromActor(FDetachmentTransformRules::KeepRelativeTransform);
		}
	}

	HeroOwner->OnUnequipped(this);
}

void AHeroEquippable::ServerEquip_Implementation(const EHand Hand)
{
	Equip(Hand);
}

bool AHeroEquippable::ServerEquip_Validate(const EHand Hand)
{
	return true;
}

void AHeroEquippable::ServerUnequip_Implementation()
{
	Unequip();
}

bool AHeroEquippable::ServerUnequip_Validate()
{
	return true;
}

void AHeroEquippable::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);
	
	HeroOwner = Cast<AHeroBase>(GetOwner());

	ensureMsgf(GetOwner() ? HeroOwner != nullptr : true, TEXT("AHeroEquippable should only be owned by a AHeroBase."));
}

void AHeroEquippable::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	InactiveStateTemplate = (InactiveStateTemplate != nullptr) ? InactiveStateTemplate : UEquippableStateInactive::StaticClass();
	InactiveState = NewObject<UEquippableState>(this, InactiveStateTemplate, TEXT("InactiveState"));
	
	StateStack.Push(InactiveState);

	ActiveStateTemplate = (ActiveStateTemplate != nullptr) ? ActiveStateTemplate : UEquippableStateActive::StaticClass();
	ActiveState = NewObject<UEquippableState>(this, ActiveStateTemplate, TEXT("ActiveState"));
}

void AHeroEquippable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AHeroEquippable, EquipStatus, COND_SkipOwner);
}