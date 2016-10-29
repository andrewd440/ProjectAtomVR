// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "HeroEquippable.h"
#include "Animation/AnimSequence.h"
#include "EquippableState.h"
#include "EquippableStateInactive.h"
#include "EquippableStateActive.h"
#include "GameFramework/PlayerController.h"
#include "Engine/ActorChannel.h"

DEFINE_LOG_CATEGORY_STATIC(LogEquippable, Log, All);

const FName AHeroEquippable::MeshComponentName = TEXT("Mesh");

AHeroEquippable::AHeroEquippable(const FObjectInitializer& ObjectInitializer/* = FObjectInitializer::Get()*/)
{
	bReplicates = true;

	Mesh = CreateAbstractDefaultSubobject<UMeshComponent>(MeshComponentName);
	RootComponent = Mesh;
}

void AHeroEquippable::BeginPlay()
{
	Super::BeginPlay();

	check(InactiveState);
	check(ActiveState);
	StateStack.Push(InactiveState);
}

void AHeroEquippable::Equip(const EHand Hand)
{
	ensureMsgf(StateStack.Top() == InactiveState, TEXT("StateStack should only have the InactiveState when equipping."));

	EquipStatus.EquippedHand = Hand;
	EquipStatus.bIsEquipped = true;

	if (HeroOwner->IsLocallyControlled())
	{
		// Enable input if locally controlled.
		EnableInput(static_cast<APlayerController*>(HeroOwner->GetController()));
	}

	if(!HeroOwner->HasAuthority())
	{
		ServerEquip(Hand);
	}
	else
	{
		// Make sure the equip update is sent to clients
		EquipStatus.ForceReplication();
	}

	StateStack.Top()->OnExitedState();
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

	if (HeroOwner->IsLocallyControlled())
	{
		// Disable input if locally controlled.
		DisableInput(static_cast<APlayerController*>(HeroOwner->GetController()));
	}

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
		StateStack.Top()->OnExitedState();
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

void AHeroEquippable::PushState(UEquippableState* InPushState)
{
	ensure(StateStack.Find(InPushState) == INDEX_NONE);
	check(InPushState);

	UE_LOG(LogEquippable, Log, TEXT("Pushed State: %s"), *InPushState->GetName());

	if (!HeroOwner->HasAuthority() && HeroOwner->IsLocallyControlled())
	{
		ServerPushState(InPushState);
	}

	StateStack.Top()->OnExitedState();
	StateStack.Push(InPushState);
	InPushState->OnEnteredState();
}

void AHeroEquippable::ServerPushState_Implementation(UEquippableState* State)
{
	PushState(State);
}

bool AHeroEquippable::ServerPushState_Validate(UEquippableState* State)
{
	return true;
}

UEquippableState* AHeroEquippable::GetCurrentState() const
{
	return StateStack.Top();
}

void AHeroEquippable::PopState(UEquippableState* InPopState)
{
	ensure(StateStack.Top() == InPopState);
	ensure(StateStack.Num() > 0);
	check(InPopState);

	UE_LOG(LogEquippable, Log, TEXT("Popped State: %s"), *InPopState->GetName());

	if (!HeroOwner->HasAuthority() && HeroOwner->IsLocallyControlled())
	{
		ServerPopState();
	}

	StateStack.Top()->OnExitedState();
	StateStack.Pop(false); // no need to shrink, it'll probably be added again
	StateStack.Top()->OnReturnedState();
}

void AHeroEquippable::ServerPopState_Implementation()
{
	PopState(StateStack.Top());
}

bool AHeroEquippable::ServerPopState_Validate()
{
	return true;
}

void AHeroEquippable::OnRep_EquipStatus()
{
	if (EquipStatus.bIsEquipped)
	{
		ensureMsgf(StateStack.Top() == InactiveState, TEXT("StateStack should only have the InactiveState when equipping."));

		PushState(ActiveState);
	}
	else
	{
		while (StateStack.Num() > 1)
		{
			StateStack.Top()->OnExitedState();
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

	// Gather all replicated states
	if (HeroOwner && HeroOwner->HasAuthority())
	{
		TArray<UObject*> Subobjects;
		GetObjectsWithOuter(this, Subobjects, false);
		for (UObject* Subobject : Subobjects)
		{
			if (Subobject->IsSupportedForNetworking())
			{
				if (UEquippableState* State = Cast<UEquippableState>(Subobject))
				{
					ReplicatedStates.Push(State);
				}
			}
		}
	}
}

void AHeroEquippable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AHeroEquippable, EquipStatus, COND_SkipOwner);
}

bool AHeroEquippable::ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);;

	for (UObject* State : ReplicatedStates)
	{
		WroteSomething |= Channel->ReplicateSubobject(State, *Bunch, *RepFlags);	
	}

	return WroteSomething;
}
