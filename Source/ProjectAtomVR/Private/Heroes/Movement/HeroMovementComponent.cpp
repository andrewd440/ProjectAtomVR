// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "HeroMovementComponent.h"

#include "AI/Navigation/AvoidanceManager.h"
#include "HMDCapsuleComponent.h"
#include "UObjectBaseUtility.h"

DEFINE_LOG_CATEGORY_STATIC(LogHeroMovement, Log, All);

UHeroMovementComponent::UHeroMovementComponent(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	MaxAcceleration = 256.f;
	MaxWalkSpeed = 240.f;

	NavAgentProps.bCanJump = false;
	NavAgentProps.bCanWalk = false;
	NavAgentProps.bCanSwim = false;
}

void UHeroMovementComponent::TeleportMove(const FVector& Destination)
{
	bWantsToTeleport = true;
	PendingTeleportDestination = Destination;
}

void UHeroMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

class FNetworkPredictionData_Client* UHeroMovementComponent::GetPredictionData_Client() const
{
	// Should only be called on client or listen server (for remote clients) in network games
	check(CharacterOwner != NULL);
	checkSlow(CharacterOwner->Role < ROLE_Authority || (CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy && GetNetMode() == NM_ListenServer));
	checkSlow(GetNetMode() == NM_Client || GetNetMode() == NM_ListenServer);

	if (!ClientPredictionData)
	{
		UHeroMovementComponent* MutableThis = const_cast<UHeroMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Hero(*this);
	}

	return ClientPredictionData;
}

class FNetworkPredictionData_Server* UHeroMovementComponent::GetPredictionData_Server() const
{
	// Should only be called on server in network games
	check(CharacterOwner != NULL);
	check(CharacterOwner->Role == ROLE_Authority);
	checkSlow(GetNetMode() < NM_Client);

	if (!ServerPredictionData)
	{
		UHeroMovementComponent* MutableThis = const_cast<UHeroMovementComponent*>(this);
		MutableThis->ServerPredictionData = new FNetworkPredictionData_Server_Hero(*this);
	}

	return ServerPredictionData;
}

void UHeroMovementComponent::ServerMove_Implementation(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode)
{
	FNetworkPredictionData_Server_Hero* ServerData = GetPredictionData_Server_Hero();
	check(ServerData);

	ServerData->ClientLocation = ClientLoc;

	Super::ServerMove_Implementation(TimeStamp, InAccel, ClientLoc, CompressedMoveFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode);
}

void UHeroMovementComponent::SmoothCorrection(const FVector& OldLocation, const FQuat& OldRotation, const FVector& NewLocation, const FQuat& NewRotation)
{
	Super::SmoothCorrection(OldLocation, OldRotation, NewLocation, NewRotation);
}

void UHeroMovementComponent::SetUpdatedComponent(USceneComponent* NewUpdatedComponent)
{
	// check that UpdatedComponent is a HMDCapsuleComponent
	if (Cast<UHMDCapsuleComponent>(NewUpdatedComponent) == NULL)
	{
		UE_LOG(LogHeroMovement, Error, TEXT("%s owned by %s must update a HMDCapsule component"), *GetName(), *GetNameSafe(NewUpdatedComponent->GetOwner()));
		return;
	}

	Super::SetUpdatedComponent(NewUpdatedComponent);
}

class FNetworkPredictionData_Server_Hero* UHeroMovementComponent::GetPredictionData_Server_Hero() const
{
	return static_cast<FNetworkPredictionData_Server_Hero*>(GetPredictionData_Server());
}

void UHeroMovementComponent::PerformMovement(float DeltaTime)
{
	Super::PerformMovement(DeltaTime);
}

void UHeroMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	bWantsToTeleport = (Flags & FSavedMove_Hero::FLAG_WantsToTeleport) != 0;
	if (bWantsToTeleport)
	{
		FNetworkPredictionData_Server_Hero* ServerData = GetPredictionData_Server_Hero();
		check(ServerData);
		PendingTeleportDestination = ServerData->ClientLocation;
	}
}

void UHeroMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

	if (bWantsToTeleport)
	{
		UE_LOG(LogHeroMovement, Log, TEXT("Teleport on %s to %s"), GetOwner()->HasAuthority() ? TEXT("Authority") : TEXT("Local"), *PendingTeleportDestination.ToString());
		UpdatedComponent->SetWorldLocation(PendingTeleportDestination);
		bWantsToTeleport = false;
		bJustTeleported = true;
	}
}

bool UHeroMovementComponent::CanDelaySendingMove(const FSavedMovePtr& NewMove)
{
	return (NewMove->GetCompressedFlags() & FSavedMove_Hero::HeroCompressedFlags::FLAG_WantsToTeleport) == 0;
}

void UHeroMovementComponent::FindFloor(const FVector& CapsuleLocation, struct FFindFloorResult& OutFloorResult, bool bZeroDelta, const FHitResult* DownwardSweepResult /*= NULL*/) const
{
	// Offset location by HMD Capsule collision offset.
	FVector CollisionOffset = FVector::ZeroVector;

	if (UHMDCapsuleComponent* const HMDCapsule = GetHMDCapsule())
	{
		CollisionOffset = HMDCapsule->GetWorldCollisionOffset();
	}

	Super::FindFloor(CapsuleLocation + CollisionOffset, OutFloorResult, bZeroDelta, DownwardSweepResult);
}

UHMDCapsuleComponent* UHeroMovementComponent::GetHMDCapsule() const
{
	return static_cast<UHMDCapsuleComponent*>(UpdatedComponent);
}

void FSavedMove_Hero::PostUpdate(ACharacter* C, EPostUpdateMode PostUpdateMode)
{
	Super::PostUpdate(C, PostUpdateMode);

	// Override Super to use UpdateComponent for location and rotation
	USceneComponent* const UpdatedComponent = C->GetCharacterMovement()->UpdatedComponent;
	SavedLocation = UpdatedComponent->GetComponentLocation();
	SavedRotation = UpdatedComponent->GetComponentRotation();
}

void FSavedMove_Hero::SetInitialPosition(ACharacter* C)
{
	Super::SetInitialPosition(C);

	// Override Super to use UpdateComponent for location and rotation
	USceneComponent* const UpdatedComponent = C->GetCharacterMovement()->UpdatedComponent;
	SavedLocation = UpdatedComponent->GetComponentLocation();
	SavedRotation = UpdatedComponent->GetComponentRotation();
}

//----------------------------------------------------------------------------------------------
// FSavedMove_Hero
//----------------------------------------------------------------------------------------------
FSavedMove_Hero::FSavedMove_Hero()
	: bWantsToTeleport(false)
{

}

bool FSavedMove_Hero::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InPawn, float MaxDelta) const
{
	if (Super::CanCombineWith(NewMove, InPawn, MaxDelta))
	{
		return true;
	}

	FSavedMove_Hero* const NewHeroMove = static_cast<FSavedMove_Hero*>(NewMove.Get());
	if (NewHeroMove->bWantsToTeleport)
	{
		// Let a teleport combine with anything. End position will always be teleport destination.
		return true;
	}

	return false;
}

void FSavedMove_Hero::Clear()
{
	Super::Clear();
	bWantsToTeleport = false;
}

uint8 FSavedMove_Hero::GetCompressedFlags() const
{
	uint8 Flags = Super::GetCompressedFlags();

	if (bWantsToTeleport)
	{
		Flags |= HeroCompressedFlags::FLAG_WantsToTeleport;
	}

	return Flags;
}

bool FSavedMove_Hero::IsImportantMove(const FSavedMovePtr& LastAckedMove) const
{
	if (Super::IsImportantMove(LastAckedMove))
	{
		return true;
	}
	
	if (bWantsToTeleport)
	{
		return true;
	}

	return false;
}

void FSavedMove_Hero::PrepMoveFor(ACharacter* C)
{
	Super::PrepMoveFor(C);

	check(Cast<UHeroMovementComponent>(C->GetMovementComponent()) && "AHeroBase requires a UHeroMovementComponent");
	UHeroMovementComponent* const MoveComponent = static_cast<UHeroMovementComponent*>(C->GetMovementComponent());
	MoveComponent->bWantsToTeleport = bWantsToTeleport;
	MoveComponent->PendingTeleportDestination = TeleportLocation;
}

void FSavedMove_Hero::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character & ClientData)
{
	Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	check(Cast<UHeroMovementComponent>(C->GetMovementComponent()) && "AHeroBase requires a UHeroMovementComponent");
	UHeroMovementComponent* const MoveComponent = static_cast<UHeroMovementComponent*>(C->GetMovementComponent());
	bWantsToTeleport = MoveComponent->bWantsToTeleport;
	TeleportLocation = MoveComponent->PendingTeleportDestination;
}

//----------------------------------------------------------------------------------------------
// FNetworkPredictionData_Client_Hero
//----------------------------------------------------------------------------------------------
FNetworkPredictionData_Client_Hero::FNetworkPredictionData_Client_Hero(const UHeroMovementComponent& ClientMovement)
	: Super(ClientMovement)
{

}

FNetworkPredictionData_Client_Hero::~FNetworkPredictionData_Client_Hero()
{

}

FSavedMovePtr FNetworkPredictionData_Client_Hero::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_Hero());
}

//----------------------------------------------------------------------------------------------
// FNetworkPredictionData_Server_Hero
//----------------------------------------------------------------------------------------------
FNetworkPredictionData_Server_Hero::FNetworkPredictionData_Server_Hero(const UHeroMovementComponent& ServerMovement)
	: Super(ServerMovement)
{

}

FNetworkPredictionData_Server_Hero::~FNetworkPredictionData_Server_Hero()
{

}
