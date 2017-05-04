// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Messages/AtomLocalMessage.h"
#include "AtomGameMessage.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UAtomGameMessage : public UAtomLocalMessage
{
	GENERATED_BODY()
		

public:
	UAtomGameMessage();

public:
	FText RoundStartMessage;

	FText RoundNearEndMessage;

	FText RoundEndMessage;	

	FText GameEndMessage;

};
