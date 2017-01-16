// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/PlayerStart.h"
#include "AtomTeamStart.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API AAtomTeamStart : public APlayerStart
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AtomTeamStart)
	int32 TeamId = 0;
};
