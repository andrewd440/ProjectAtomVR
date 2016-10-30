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

	bReturnToLoadout = true;
}

void AHeroEquippable::BeginPlay()
{
	Super::BeginPlay();

	for (auto& State : EquippableStates)
	{
		State->BeginPlay();
	}

	check(InactiveState);
	check(ActiveState);
	StateStack.Push(InactiveState);
}

void AHeroEquippable::Equip(const EHand Hand, const EEquipType EquipType)
{
	ensureMsgf(StateStack.Top() == InactiveState, TEXT("StateStack should only have the InactiveState when equipping."));
	
	EquipStatus.EquippedHand = Hand;
	EquipStatus.EquipType = EquipType;
	EquipStatus.bIsEquipped = true;
	EquipStatus.ForceReplication(); // Make sure the equip update is sent to clients

	if (HeroOwner->IsLocallyControlled())
	{
		// Enable input if locally controlled.
		EnableInput(static_cast<APlayerController*>(HeroOwner->GetController()));
	}

	if (EquipType == EEquipType::Normal && !HeroOwner->HasAuthority())
	{
		ServerEquip(Hand);
	}

	StateStack.Top()->OnExitedState();
	StateStack.Push(ActiveState);
	ActiveState->OnEnteredState();
}

bool AHeroEquippable::CanEquip(const EHand Hand) const
{
	return !IsEquipped() && GetWorld()->GetTimeSeconds() > UnequipTimeStamp + ReuseDelay;
}

void AHeroEquippable::Unequip(const EEquipType EquipType)
{
	ensure(EquipStatus.bIsEquipped);
	EquipStatus.bIsEquipped = false;
	EquipStatus.EquipType = EquipType;

	// Make sure the equip update is sent to clients
	EquipStatus.ForceReplication();

	if (HeroOwner->IsLocallyControlled())
	{
		// Disable input if locally controlled.
		DisableInput(static_cast<APlayerController*>(HeroOwner->GetController()));
	}

	if (EquipType == EEquipType::Normal && !HeroOwner->HasAuthority())
	{
		ServerUnequip();
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

void AHeroEquippable::SetCanReturnToLoadout(bool bCanReturn)
{
	if (bCanReturn != bReturnToLoadout)
	{
		bReturnToLoadout = bCanReturn;
		OnCanReturnToLoadoutChanged.Broadcast();
	}
}

bool AHeroEquippable::CanReturnToLoadout() const
{
	return bReturnToLoadout;
}

void AHeroEquippable::SetLoadoutAttachment(USceneComponent* AttachComponent, FName AttachSocket)
{
	LoadoutAttachComponent = AttachComponent;
	LoadoutAttachSocket = AttachSocket;
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
	// The server could already be in the requested state, so check first.
	if (StateStack.Top() != State)
	{
		PushState(State);
	}
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
	if (EquipStatus.EquipType == EEquipType::Normal)
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

	// Set return storage before attaching to hero
	SetLoadoutAttachment(Mesh->GetAttachParent(), Mesh->GetAttachSocketName());
	AttachToComponent(AttachHand, FAttachmentTransformRules::SnapToTargetNotIncludingScale, HandAttachSocket); 

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

	if (LoadoutAttachComponent && bReturnToLoadout)
	{
		AttachToComponent(LoadoutAttachComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, LoadoutAttachSocket);
	}
	else
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
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

	// Gather all states
	TArray<UObject*> Subobjects;
	GetObjectsWithOuter(this, Subobjects, false);
	for (UObject* Subobject : Subobjects)
	{
		if (UEquippableState* State = Cast<UEquippableState>(Subobject))
		{
			EquippableStates.Push(State);
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

	// All states are required to replicate
	for (UObject* State : EquippableStates)
	{
		WroteSomething |= Channel->ReplicateSubobject(State, *Bunch, *RepFlags);	
	}

	return WroteSomething;
}
