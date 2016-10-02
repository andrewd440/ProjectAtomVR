// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "HeroMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UHeroMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
	UHeroMovementComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void TeleportMove(const FVector& Destination);

	class FNetworkPredictionData_Server_Hero* GetPredictionData_Server_Hero() const;

	/** UCharacterMovementComponent Interface Begin */
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual class FNetworkPredictionData_Server* GetPredictionData_Server() const override;
	virtual void ServerMove_Implementation(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode) override;
	virtual void SmoothCorrection(const FVector& OldLocation, const FQuat& OldRotation, const FVector& NewLocation, const FQuat& NewRotation) override;

protected:
	virtual void PerformMovement(float DeltaTime) override;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	virtual bool CanDelaySendingMove(const FSavedMovePtr& NewMove) override;
	/** UCharacterMovementComponent Interface End */

protected:
	friend class FSavedMove_Hero;
	uint32 bWantsToTeleport : 1;
	FVector PendingTeleportDestination;
};


class PROJECTATOMVR_API FSavedMove_Hero : public FSavedMove_Character
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
	FSavedMove_Hero();

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

class PROJECTATOMVR_API FNetworkPredictionData_Client_Hero : public FNetworkPredictionData_Client_Character
{
	using Super = FNetworkPredictionData_Client_Character;

public:
	FNetworkPredictionData_Client_Hero(const UHeroMovementComponent& ClientMovement);
	virtual ~FNetworkPredictionData_Client_Hero();

	/** FNetworkPredictionData_Client_Character Interface Begin */
	virtual FSavedMovePtr AllocateNewMove() override;
	/** FNetworkPredictionData_Client_Character Interface End */
};

class PROJECTATOMVR_API FNetworkPredictionData_Server_Hero : public FNetworkPredictionData_Server_Character
{
	using Super = FNetworkPredictionData_Server_Character;

public:
	FNetworkPredictionData_Server_Hero(const UHeroMovementComponent& ServerMovement);
	virtual ~FNetworkPredictionData_Server_Hero();

public:
	FVector ClientLocation = FVector::ZeroVector;
};