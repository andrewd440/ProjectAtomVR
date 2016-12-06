// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "HeroEquippable.h"
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

const FName AHeroEquippable::MeshComponentName = TEXT("Mesh");
const FName AHeroEquippable::InactiveStateName = TEXT("InactiveState");
const FName AHeroEquippable::ActiveStateName = TEXT("ActiveState");

AHeroEquippable::AHeroEquippable(const FObjectInitializer& ObjectInitializer/* = FObjectInitializer::Get()*/)
{
	bReplicates = true;
	bReplicatesAttachment = false;

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
	SecondaryHandGripTrigger->SetCollisionObjectType(CollisionChannelAliases::HandTrigger);
	SecondaryHandGripTrigger->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SecondaryHandGripTrigger->SetCollisionResponseToChannel(CollisionChannelAliases::HeroHand, ECollisionResponse::ECR_Overlap);

	SecondaryHandGripTrigger->OnComponentBeginOverlap.AddDynamic(this, &AHeroEquippable::OnBeginOverlapSecondaryHandTrigger);
	SecondaryHandGripTrigger->OnComponentEndOverlap.AddDynamic(this, &AHeroEquippable::OnEndOverlapSecondaryHandTrigger);

	bIsSecondaryHandAttachmentAllowed = true;
	bReturnToLoadout = true;
}

void AHeroEquippable::BeginPlay()
{
	Super::BeginPlay();

	check(InactiveState);
	check(ActiveState);
	StateStack.Push(InactiveState);

	// May already be equipped through replication. Call OnRep now since a call 
	// prior to BeginPlay will be ignored.
	if (EquipStatus.bIsEquipped)
	{
		OnRep_EquipStatus();
	}

	for (auto State : EquippableStates)
	{
		State->BeginPlay();
	}
}

void AHeroEquippable::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AHeroEquippable::Equip(const EHand Hand, const EEquipType EquipType)
{
	check(StateStack.Num() > 0 && StateStack.Top() == InactiveState && "StateStack should only have the InactiveState when equipping.");
	
	EquipStatus.EquippedHand = Hand;
	EquipStatus.EquipType = EquipType;
	EquipStatus.bIsEquipped = true;
	EquipStatus.ForceReplication(); // Make sure the equip update is sent to clients
	
	if (HeroOwner->IsLocallyControlled())
	{
		// Enable input if locally controlled.
		EnableInput(static_cast<APlayerController*>(HeroOwner->GetController()));
		SetupInputComponent(InputComponent);
	}

	if (EquipType == EEquipType::Normal && !HeroOwner->HasAuthority())
	{
		ServerEquip(Hand);
	}

	StateStack.Top()->OnExitedState();
	StateStack.Push(ActiveState);
	ActiveState->OnStatePushed();
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

	if (EquipType == EEquipType::Normal && !HeroOwner->HasAuthority())
	{
		ServerUnequip();
	}

	while (StateStack.Num() > 1)
	{		
		UEquippableState* PoppedState = StateStack.Pop(false);
		PoppedState->OnStatePopped();
	}

	// Notify inactive state of entered event
	ensure(StateStack.Top() == InactiveState);
	StateStack.Top()->OnEnteredState();

	if (HeroOwner->IsLocallyControlled())
	{
		// Disable input if locally controlled.
		check(InputComponent);
		DisableInput(static_cast<APlayerController*>(HeroOwner->GetController()));
		InputComponent->DestroyComponent();
		InputComponent = nullptr;
	}
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

	//UE_LOG(LogEquippable, Log, TEXT("Pushed State: %s"), *InPushState->GetName());

	if (!HeroOwner->HasAuthority() && HeroOwner->IsLocallyControlled())
	{
		ServerPushState(InPushState);
	}

	UEquippableState* const LastState = StateStack.Top();
	StateStack.Push(InPushState);
	LastState->OnExitedState();
	InPushState->OnStatePushed();
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

	//UE_LOG(LogEquippable, Log, TEXT("Popped State: %s"), *InPopState->GetName());

	if (!HeroOwner->HasAuthority() && HeroOwner->IsLocallyControlled())
	{
		ServerPopState(InPopState);
	}

	UEquippableState* PoppedState = StateStack.Pop(false); // no need to shrink, it'll probably be added again
	PoppedState->OnStatePopped();
	StateStack.Top()->OnEnteredState();
}

void AHeroEquippable::ServerPopState_Implementation(UEquippableState* InPopState)
{
	// The server could already be in the requested state, so check first.
	if (StateStack.Top() == InPopState)
	{
		PopState(InPopState);
	}
}

bool AHeroEquippable::ServerPopState_Validate(UEquippableState* InPopState)
{
	return true;
}

void AHeroEquippable::OnRep_EquipStatus()
{
	if (EquipStatus.EquipType == EEquipType::Normal && HasActorBegunPlay())
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
				UEquippableState* PoppedState = StateStack.Pop(false);
				PoppedState->OnStatePopped();
			}

			// Notify inactive state of entered event
			check(StateStack.Num() > 0 && StateStack.Top() == InactiveState);
			StateStack.Top()->OnEnteredState();
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
	// Set return storage before attaching to hero
	SetLoadoutAttachment(Mesh->GetAttachParent(), Mesh->GetAttachSocketName());
	
	USkeletalMeshComponent* const AttachHand = HeroOwner->GetHandAttachmentComponent(EquipStatus.EquippedHand);

	FString AttachSocket;
	PrimaryHandAttachSocket.ToString(AttachSocket);
	AttachSocket += (EquipStatus.EquippedHand == EHand::Left) ? TEXT("_Left") : TEXT("_Right");
	AttachToComponent(AttachHand, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName{ *AttachSocket });

	HeroOwner->PlayHandAnimation(EquipStatus.EquippedHand, AnimHandEquip);

	if (EquipSound)
	{
		UGameplayStatics::SpawnSoundAttached(EquipSound, Mesh);
	}

	if (bIsSecondaryHandAttachmentAllowed)
	{
		SecondaryHandGripTrigger->bGenerateOverlapEvents = true;
	}	

	HeroOwner->OnEquipped(this, EquipStatus.EquippedHand);

	OnEquippedStatusChangedUI.ExecuteIfBound();
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

	if (EquipSound)
	{
		UGameplayStatics::SpawnSoundAttached(EquipSound, Mesh);
	}	

	SecondaryHandGripTrigger->bGenerateOverlapEvents = false;

	HeroOwner->StopHandAnimation(EquipStatus.EquippedHand, AnimHandEquip);
	HeroOwner->OnUnequipped(this, EquipStatus.EquippedHand);

	OnEquippedStatusChangedUI.ExecuteIfBound();
}

TSubclassOf<class AEquippableUIActor> AHeroEquippable::GetUIActor() const
{
	return EquippableUI;
}

void AHeroEquippable::SetReplicatesAttachment(bool bShouldReplicate)
{
	bReplicatesAttachment = bShouldReplicate;
}

void AHeroEquippable::SetupInputComponent(UInputComponent* InInputComponent)
{
	check(InInputComponent);
}

USceneComponent* AHeroEquippable::GetOffsetTarget() const
{
	return HeroOwner->GetHandMeshTarget(EquipStatus.EquippedHand);
}

void AHeroEquippable::GetOriginalOffsetTargetLocationAndRotation(FVector& LocationOut, FRotator& RotationOut) const
{
	HeroOwner->GetDefaultHandMeshLocationAndRotation(EquipStatus.EquippedHand, LocationOut, RotationOut);
}

void AHeroEquippable::OnBeginOverlapSecondaryHandTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	const EHand SecondaryHand = !EquipStatus.EquippedHand;

	if (EquipStatus.bIsEquipped &&
		HeroOwner->GetHandTrigger(SecondaryHand) == OtherComp && // Is it the other hand and is it empty	
		HeroOwner->GetEquippable(SecondaryHand) == nullptr)
	{
		USceneComponent* const HandTarget = HeroOwner->GetHandMeshTarget(SecondaryHand);
		HandTarget->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, (SecondaryHand == EHand::Left) ? SecondaryHandAttachLeftSocket : SecondaryHandAttachRightSocket);

		bIsSecondaryHandAttached = true;

		HeroOwner->PlayHandAnimation(!EquipStatus.EquippedHand, AnimSecondaryHandEquip);
		HeroOwner->OnEquipped(this, SecondaryHand);
	}
}

void AHeroEquippable::OnEndOverlapSecondaryHandTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	const EHand SecondaryHand = !EquipStatus.EquippedHand;

	ensure(!bIsSecondaryHandAttached || HeroOwner->GetHandTrigger(SecondaryHand) != OtherComp || HeroOwner->GetEquippable(SecondaryHand) == this);

	if (bIsSecondaryHandAttached && EquipStatus.bIsEquipped &&
		HeroOwner->GetHandTrigger(SecondaryHand) == OtherComp && // Is it the other hand and is it holding this	
		HeroOwner->GetEquippable(SecondaryHand) == this)
	{
		bIsSecondaryHandAttached = false;
		HeroOwner->StopHandAnimation(!EquipStatus.EquippedHand, AnimSecondaryHandEquip);
		HeroOwner->OnUnequipped(this, !EquipStatus.EquippedHand);
	}
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

	SecondaryHandGripTrigger->bGenerateOverlapEvents = false;		
}

void AHeroEquippable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AHeroEquippable, EquipStatus, COND_SkipOwner);
}

void AHeroEquippable::PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	// Override AttachmentReplication replication to flag
	static UProperty* spAttachmentReplication = GetReplicatedProperty(StaticClass(), AActor::StaticClass(), TEXT("AttachmentReplication"));
	for ( int32 i = 0; i < spAttachmentReplication->ArrayDim; i++ )
	{																						
		ChangedPropertyTracker.SetCustomIsActiveOverride(spAttachmentReplication->RepIndex + i, bReplicatesAttachment );
	}
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

void AHeroEquippable::GetSubobjectsWithStableNamesForNetworking(TArray<UObject*>& ObjList)
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