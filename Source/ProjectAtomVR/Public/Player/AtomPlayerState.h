// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/PlayerState.h"
#include "AtomPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API AAtomPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:

	/** APlayerState Interface Begin */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	/** APlayerState Interface End */	

protected:
	UFUNCTION()
	virtual void OnRep_TeamId();

public:
	UPROPERTY(Transient, Replicated, BlueprintReadWrite, Category = AtomPlayerState)
	int32 Kills = 0;

	UPROPERTY(Transient, Replicated, BlueprintReadWrite, Category = AtomPlayerState)
	int32 Deaths = 0;

	UPROPERTY(Transient, ReplicatedUsing=OnRep_TeamId, BlueprintReadWrite, Category = AtomPlayerState)
	int32 TeamId = -1;
};
