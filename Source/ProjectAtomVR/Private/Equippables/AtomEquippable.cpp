// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomEquippable.h"
#include "Animation/AnimSequence.h"
#include "EquippableState.h"
#include "EquippableStateInactive.h"
#include "EquippableStateActive.h"
#include "NetMotionControllerComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/ActorChannel.h"

DEFINE_LOG_CATEGORY_STATIC(LogEquippable, Log, All);

namespace 
{
	static const FName SecondaryHandAttachLeftSocket{ TEXT("SecondaryHandAttachLeft") };
	static const FName SecondaryHandAttachRightSocket{ TEXT("SecondaryHandAttachRight") };
}

const FName AAtomEquippable::MeshComponentName = TEXT("Mesh");
const FName AAtomEquippable::InactiveStateName = TEXT("InactiveState");
const FName AAtomEquippable::ActiveStateName = TEXT("ActiveState");

AAtomEquippable::AAtomEquippable(const FObjectInitializer& ObjectInitializer/* = FObjectInitializer::Get()*/)
{
	bReplicates = true;
	bReplicatesAttachment = false;
	bIsSimulatingReplication = false;

	LoadoutType = ELoadoutType::Item;

	Mesh = CreateAbstractDefaultSubobject<UMeshComponent>(MeshComponentName);	
	RootComponent = Mesh;

	InactiveState = CreateDefaultSubobject<UEquippableStateInactive>(InactiveStateName);
	ActiveState = CreateDefaultSubobject<UEquippableStateActive>(ActiveStateName);

	// Setup secondary attach trigger
	SecondaryHandGripTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("SecondaryHandGripTrigger"));
	SecondaryHandGripTrigger->SetIsReplicated(false);
	SecondaryHandGripTrigger->bGenerateOverlapEvents = false;
	SecondaryHandGripTrigger->SetupAttachment(Mesh);

	// Overlap only Hero hand
	SecondaryHandGripTrigger->SetCollisionObjectType(AtomCollisionChannels::HandTrigger);
	SecondaryHandGripTrigger->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SecondaryHandGripTrigger->SetCollisionResponseToChannel(AtomCollisionChannels::HeroHand, ECollisionResponse::ECR_Overlap);

	SecondaryHandGripTrigger->OnComponentBeginOverlap.AddDynamic(this, &AAtomEquippable::OnBeginOverlapSecondaryHandTrigger);
	SecondaryHandGripTrigger->OnComponentEndOverlap.AddDynamic(this, &AAtomEquippable::OnEndOverlapSecondaryHandTrigger);

	bIsSecondaryHandAttachmentAllowed = true;
}

void AAtomEquippable::BeginPlay()
{
	Super::BeginPlay();

	check(InactiveState);
	check(ActiveState);
	StateStack.Push(InactiveState);

	// May already be equipped through replication. Call OnRep now since a call 
	// prior to BeginPlay will be ignored.
	if (EquipStatus.State != EEquipState::Unequipped)
	{
		OnRep_EquipStatus();
	}

	for (auto State : EquippableStates)
	{
		State->BeginPlay();
	}
}

void AAtomEquippable::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AAtomEquippable::Equip(const EHand Hand, const EEquipType EquipType)
{
	check(StateStack.Num() > 0 && StateStack.Top() == InactiveState && "StateStack should only have the InactiveState when equipping.");
	
	EquipStatus.Hand = Hand;
	EquipStatus.State = EEquipState::Equipped;
	
	if (CharacterOwner->IsLocallyControlled())
	{
		// Enable input if locally controlled.
		EnableInput(CastChecked<APlayerController>(CharacterOwner->GetController()));
		SetupInputComponent(InputComponent);
	}

	if (!bIsSimulatingReplication)
	{
		if (EquipType == EEquipType::Normal)
		{
			if (CharacterOwner->HasAuthority())
			{
				ReplicatedEquipStatus = EquipStatus;
				ReplicatedEquipStatus.ForceReplication();
			}
			else
			{
				ServerEquip(Hand);
			}
		}
	}

	StateStack.Top()->OnExitedState();
	StateStack.Push(ActiveState);
	ActiveState->OnStatePushed();
}

bool AAtomEquippable::CanEquip(const EHand Hand) const
{
	return !IsEquipped() && GetWorld()->GetTimeSeconds() > UnequipTimeStamp + ReuseDelay;
}

void AAtomEquippable::Unequip(const EEquipType EquipType)
{
	ensure(EquipStatus.State == EEquipState::Equipped);
	EquipStatus.State = EEquipState::Unequipped;

	if (!bIsSimulatingReplication)
	{
		if (EquipType == EEquipType::Normal)
		{
			if (CharacterOwner->HasAuthority())
			{
				ReplicatedEquipStatus = EquipStatus;
				ReplicatedEquipStatus.ForceReplication();
			}
			else
			{
				ServerUnequip();
			}
		}
	}

	while (StateStack.Num() > 1)
	{		
		UEquippableState* PoppedState = StateStack.Pop(false);
		UE_LOG(LogEquippable, Log, TEXT("Popped State: %s"), *PoppedState->GetName());
		PoppedState->OnStatePopped();
	}

	// Notify inactive state of entered event
	ensure(StateStack.Top() == InactiveState);
	StateStack.Top()->OnEnteredState();

	if (CharacterOwner->IsLocallyControlled())
	{
		// Disable input if locally controlled.
		check(InputComponent);
		DisableInput(CastChecked<APlayerController>(CharacterOwner->GetController()));
		InputComponent->DestroyComponent();
		InputComponent = nullptr;
	}
}

void AAtomEquippable::Drop()
{
	if (HasAuthority())
	{
		CharacterOwner->DiscardFromLoadout(this);
	}

	Unequip(EEquipType::Deferred);

	EquipStatus.State = EEquipState::Dropped;

	if (!bIsSimulatingReplication)
	{
		if (CharacterOwner->HasAuthority())
		{
			ReplicatedEquipStatus = EquipStatus;
			ReplicatedEquipStatus.ForceReplication();
		}
		else
		{
			ServerDrop();
		}
	}

	Mesh->SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);
	SetActorEnableCollision(true);

	Mesh->SetSimulatePhysics(true);
	SetLifeSpan(10.f);
}

void AAtomEquippable::UpdateCharacterAttachment()
{
	USkeletalMeshComponent* const AttachHand = CharacterOwner->GetHandAttachmentComponent(EquipStatus.Hand);

	FString AttachSocket;
	PrimaryHandAttachSocket.ToString(AttachSocket);
	AttachSocket += (EquipStatus.Hand == EHand::Left) ? TEXT("_l") : TEXT("_r");
	AttachToComponent(AttachHand, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName{ *AttachSocket });

	CharacterOwner->PlayHandAnimation(EquipStatus.Hand, AnimHandEquip);
}

void AAtomEquippable::PushState(UEquippableState* InPushState)
{
	ensure(StateStack.Find(InPushState) == INDEX_NONE);
	check(InPushState);

	UE_LOG(LogEquippable, Log, TEXT("Pushed State: %s"), *InPushState->GetName());

	if (!CharacterOwner->HasAuthority() && CharacterOwner->IsLocallyControlled())
	{
		ServerPushState(InPushState);
	}

	UEquippableState* const LastState = StateStack.Top();
	StateStack.Push(InPushState);
	LastState->OnExitedState();
	InPushState->OnStatePushed();
}

void AAtomEquippable::ServerPushState_Implementation(UEquippableState* State)
{
	// The server could already be in the requested state, so check first.
	if (StateStack.Top() != State)
	{
		PushState(State);
	}
}

bool AAtomEquippable::ServerPushState_Validate(UEquippableState* State)
{
	return true;
}

UEquippableState* AAtomEquippable::GetCurrentState() const
{
	return StateStack.Top();
}

void AAtomEquippable::PopState(UEquippableState* InPopState)
{
	ensure(StateStack.Top() == InPopState);
	ensure(StateStack.Num() > 0);
	check(InPopState);

	UE_LOG(LogEquippable, Log, TEXT("Popped State: %s"), *InPopState->GetName());

	if (!CharacterOwner->HasAuthority() && CharacterOwner->IsLocallyControlled())
	{
		ServerPopState(InPopState);
	}

	UEquippableState* PoppedState = StateStack.Pop(false); // no need to shrink, it'll probably be added again
	PoppedState->OnStatePopped();
	StateStack.Top()->OnEnteredState();
}

void AAtomEquippable::ServerPopState_Implementation(UEquippableState* InPopState)
{
	// The server could already be in the requested state, so check first.
	if (StateStack.Top() == InPopState)
	{
		PopState(InPopState);
	}
}

bool AAtomEquippable::ServerPopState_Validate(UEquippableState* InPopState)
{
	return true;
}

void AAtomEquippable::ServerDrop_Implementation()
{
	Drop();
}

bool AAtomEquippable::ServerDrop_Validate()
{
	return true;
}

void AAtomEquippable::OnRep_EquipStatus()
{
	if (HasActorBegunPlay())
	{
		bIsSimulatingReplication = true;

		if (ReplicatedEquipStatus.State == EEquipState::Equipped)
		{
			ensureMsgf(StateStack.Top() == InactiveState, TEXT("StateStack should only have the InactiveState when equipping."));

			Equip(ReplicatedEquipStatus.Hand);
		}
		else if (ReplicatedEquipStatus.State == EEquipState::Unequipped)
		{
			Unequip();
		}
		else
		{
			Drop();
		}

		bIsSimulatingReplication = false;
	}
}

void AAtomEquippable::OnRep_Owner()
{
	Super::OnRep_Owner();

	CharacterOwner = Cast<AAtomCharacter>(GetOwner());

	ensureMsgf(GetOwner() ? CharacterOwner != nullptr : true , TEXT("AHeroEquippable should only be owned by a AHeroBase."));
}

void AAtomEquippable::OnEquipped()
{		
	UpdateCharacterAttachment();

	if (EquipSound)
	{
		UGameplayStatics::SpawnSoundAttached(EquipSound, Mesh);
	}

	if (bIsSecondaryHandAttachmentAllowed)
	{
		SecondaryHandGripTrigger->bGenerateOverlapEvents = true;
	}	

	CharacterOwner->OnEquipped(this, EquipStatus.Hand);

	OnEquippedStatusChangedUI.ExecuteIfBound();
}

void AAtomEquippable::OnUnequipped()
{
	UnequipTimeStamp = GetWorld()->GetTimeSeconds();

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	if (bIsSecondaryHandAttached)
	{
		check(CharacterOwner->GetEquippable(!EquipStatus.Hand) == this && "Other hand should have this equipped if second hand is attached.");

		bIsSecondaryHandAttached = false;
		CharacterOwner->StopHandAnimation(!EquipStatus.Hand, AnimSecondaryHandEquip);
		CharacterOwner->OnUnequipped(this, !EquipStatus.Hand);
	}

	if (EquipSound)
	{
		UGameplayStatics::SpawnSoundAttached(EquipSound, Mesh);
	}	

	SecondaryHandGripTrigger->bGenerateOverlapEvents = false;

	CharacterOwner->StopHandAnimation(EquipStatus.Hand, AnimHandEquip);
	CharacterOwner->OnUnequipped(this, EquipStatus.Hand);

	OnEquippedStatusChangedUI.ExecuteIfBound();
}

TSubclassOf<class AEquippableHUDActor> AAtomEquippable::GetHUDActor() const
{
	return HUDActor;
}

void AAtomEquippable::SetReplicatesAttachment(bool bShouldReplicate)
{
	bReplicatesAttachment = bShouldReplicate;
}

ELoadoutType AAtomEquippable::GetLoadoutType() const
{
	return LoadoutType;
}

void AAtomEquippable::SetupInputComponent(UInputComponent* InInputComponent)
{
	check(InInputComponent);
}

USceneComponent* AAtomEquippable::GetOffsetTarget() const
{
	return CharacterOwner->GetHandMeshTarget(EquipStatus.Hand);
}

void AAtomEquippable::GetOriginalOffsetTargetLocationAndRotation(FVector& LocationOut, FRotator& RotationOut) const
{
	CharacterOwner->GetDefaultHandMeshLocationAndRotation(EquipStatus.Hand, LocationOut, RotationOut);
}

void AAtomEquippable::OnBeginOverlapSecondaryHandTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	const EHand SecondaryHand = !EquipStatus.Hand;

	if (EquipStatus.State == EEquipState::Equipped &&
		CharacterOwner->GetHandTrigger(SecondaryHand) == OtherComp && // Is it the other hand and is it empty	
		CharacterOwner->GetEquippable(SecondaryHand) == nullptr)
	{
		USceneComponent* const HandTarget = CharacterOwner->GetHandMeshTarget(SecondaryHand);
		HandTarget->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, 
			(SecondaryHand == EHand::Left) ? SecondaryHandAttachLeftSocket : SecondaryHandAttachRightSocket);

		bIsSecondaryHandAttached = true;

		CharacterOwner->PlayHandAnimation(!EquipStatus.Hand, AnimSecondaryHandEquip);
		CharacterOwner->OnEquipped(this, SecondaryHand);
	}
}

void AAtomEquippable::OnEndOverlapSecondaryHandTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	const EHand SecondaryHand = !EquipStatus.Hand;

	ensure(!bIsSecondaryHandAttached || 
		CharacterOwner->GetHandTrigger(SecondaryHand) != OtherComp || 
		CharacterOwner->GetEquippable(SecondaryHand) == this);

	if (bIsSecondaryHandAttached && 
		EquipStatus.State == EEquipState::Equipped &&
		CharacterOwner->GetHandTrigger(SecondaryHand) == OtherComp && // Is it the other hand and is it holding this	
		CharacterOwner->GetEquippable(SecondaryHand) == this)
	{
		bIsSecondaryHandAttached = false;
		CharacterOwner->StopHandAnimation(!EquipStatus.Hand, AnimSecondaryHandEquip);
		CharacterOwner->OnUnequipped(this, !EquipStatus.Hand);
	}
}

void AAtomEquippable::ServerEquip_Implementation(const EHand Hand)
{
	Equip(Hand);
}

bool AAtomEquippable::ServerEquip_Validate(const EHand Hand)
{
	return true;
}

void AAtomEquippable::ServerUnequip_Implementation()
{
	Unequip();
}

bool AAtomEquippable::ServerUnequip_Validate()
{
	return true;
}

void AAtomEquippable::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);
	
	CharacterOwner = Cast<AAtomCharacter>(GetOwner());

	ensureMsgf(GetOwner() ? CharacterOwner != nullptr : true, TEXT("AHeroEquippable should only be owned by a AHeroBase."));
}

void AAtomEquippable::PostInitializeComponents()
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

	SecondaryHandGripTrigger->bGenerateOverlapEvents = false;		
}

void AAtomEquippable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AAtomEquippable, ReplicatedEquipStatus, COND_SkipOwner);
}

void AAtomEquippable::PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	// Override AttachmentReplication replication to flag
	static UProperty* spAttachmentReplication = GetReplicatedProperty(StaticClass(), AActor::StaticClass(), TEXT("AttachmentReplication"));
	for ( int32 i = 0; i < spAttachmentReplication->ArrayDim; i++ )
	{																						
		ChangedPropertyTracker.SetCustomIsActiveOverride(spAttachmentReplication->RepIndex + i, bReplicatesAttachment );
	}
}

void AAtomEquippable::Destroyed()
{
	if (StateStack.Num() > 0 && StateStack.Top())
	{
		StateStack.Top()->Deactivate();
	}
}

bool AAtomEquippable::ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);;

	// All states are required to replicate
	for (UObject* State : EquippableStates)
	{
		WroteSomething |= Channel->ReplicateSubobject(State, *Bunch, *RepFlags);	
	}

	return WroteSomething;
}

void AAtomEquippable::GetSubobjectsWithStableNamesForNetworking(TArray<UObject*>& ObjList)
{
	Super::GetSubobjectsWithStableNamesForNetworking(ObjList);

	// For experimenting with replicating ALL stably named components initially
	for (UEquippableState* State : EquippableStates)
	{		
		if (State && State->IsNameStableForNetworking())
		{
			ObjList.Add(State);
			State->GetSubobjectsWithStableNamesForNetworking(ObjList);
		}
	}

	// Sort to keep lists consistent on server and clients
	Sort(ObjList.GetData(), ObjList.Num(), [](const UObject& A, const UObject& B) {return A.GetName() < B.GetName(); });
}