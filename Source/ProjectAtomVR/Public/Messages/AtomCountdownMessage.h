// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Messages/AtomLocalMessage.h"
#include "AtomCountdownMessage.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UAtomCountdownMessage : public UAtomLocalMessage
{
	GENERATED_BODY()
	
public:	
	UAtomCountdownMessage();

public:
	FText CountdownMessage;
};
