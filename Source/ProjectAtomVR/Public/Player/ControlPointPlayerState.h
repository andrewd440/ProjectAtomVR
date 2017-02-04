// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Player/AtomPlayerState.h"
#include "ControlPointPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API AControlPointPlayerState : public AAtomPlayerState
{
	GENERATED_BODY()

public:

	/** AAtomPlayerState Interface Begin */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	/** AAtomPlayerState Interface End */

public:
	UPROPERTY(BlueprintReadOnly, Replicated, Category = ControlPointPlayerState)
	int32 PointsCaptured = 0; // Number of control points captured this game

	UPROPERTY(BlueprintReadOnly, Replicated, Category = ControlPointPlayerState)
	int32 AttackKills = 0; // Number of kills while attacking a control points this game

	UPROPERTY(BlueprintReadOnly, Replicated, Category = ControlPointPlayerState)
	int32 DefenseKills = 0; // Number of kills while defending a control points this game
};