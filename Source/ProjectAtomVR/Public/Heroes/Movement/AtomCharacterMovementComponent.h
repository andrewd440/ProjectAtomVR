// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "AtomCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UAtomCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
	UAtomCharacterMovementComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void TeleportMove(const FVector& Destination);

	class FNetworkPredictionData_Server_AtomCharacter* GetPredictionData_Server_Hero() const;

protected:
	class UHMDCapsuleComponent* GetHMDCapsule() const;

	/** UCharacterMovementComponent Interface Begin */
public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual class FNetworkPredictionData_Server* GetPredictionData_Server() const override;
	virtual void ServerMove_Implementation(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode) override;
	virtual void SmoothCorrection(const FVector& OldLocation, const FQuat& OldRotation, const FVector& NewLocation, const FQuat& NewRotation) override;
	virtual void SetUpdatedComponent(USceneComponent* NewUpdatedComponent) override;

protected:
	virtual void PerformMovement(float DeltaTime) override;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	virtual bool CanDelaySendingMove(const FSavedMovePtr& NewMove) override;
	virtual void FindFloor(const FVector& CapsuleLocation, struct FFindFloorResult& OutFloorResult, bool bZeroDelta, const FHitResult* DownwardSweepResult = NULL) const override;
	/** UCharacterMovementComponent Interface End */

	/** UActorComponent Interface Begin */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	/** UActorComponent Interface End */

protected:
	friend class FSavedMove_AtomCharacter;
	uint32 bWantsToTeleport : 1;

	UPROPERTY(ReplicatedUsing=OnRep_PendingTeleportDestination)
	FVector PendingTeleportDestination;

private:
	UFUNCTION()
	void OnRep_PendingTeleportDestination();
};


class PROJECTATOMVR_API FSavedMove_AtomCharacter : public FSavedMove_Character
{
	using Super = FSavedMove_Character;

public:
	enum HeroCompressedFlags
	{
		FLAG_WantsToTeleport = CompressedFlags::FLAG_Custom_0,
	};


	virtual void PostUpdate(ACharacter* C, EPostUpdateMode PostUpdateMode) override;


	virtual void SetInitialPosition(ACharacter* C) override;

public:
	FSavedMove_AtomCharacter();

	/** FSavedMove_Character Interface Begin */
	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InPawn, float MaxDelta) const override;
	virtual void Clear() override;
	virtual uint8 GetCompressedFlags() const override;
	virtual bool IsImportantMove(const FSavedMovePtr& LastAckedMove) const override;
	virtual void PrepMoveFor(ACharacter* C) override;
	virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character & ClientData) override;
	/** FSavedMove_Character Interface End */

public:
	uint32 bWantsToTeleport : 1; // If this move is a teleport
	FVector TeleportLocation; // Only valid if bWantsToTeleport
};

class PROJECTATOMVR_API FNetworkPredictionData_Client_AtomCharacter : public FNetworkPredictionData_Client_Character
{
	using Super = FNetworkPredictionData_Client_Character;

public:
	FNetworkPredictionData_Client_AtomCharacter(const UAtomCharacterMovementComponent& ClientMovement);
	virtual ~FNetworkPredictionData_Client_AtomCharacter();

	/** FNetworkPredictionData_Client_Character Interface Begin */
	virtual FSavedMovePtr AllocateNewMove() override;
	/** FNetworkPredictionData_Client_Character Interface End */
};

class PROJECTATOMVR_API FNetworkPredictionData_Server_AtomCharacter : public FNetworkPredictionData_Server_Character
{
	using Super = FNetworkPredictionData_Server_Character;

public:
	FNetworkPredictionData_Server_AtomCharacter(const UAtomCharacterMovementComponent& ServerMovement);
	virtual ~FNetworkPredictionData_Server_AtomCharacter();

public:
	FVector ClientLocation = FVector::ZeroVector;
};