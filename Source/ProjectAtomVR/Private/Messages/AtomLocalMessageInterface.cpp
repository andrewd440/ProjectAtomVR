// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomLocalMessageInterface.h"


// This function does not need to be modified.
UAtomLocalMessageInterface::UAtomLocalMessageInterface(const class FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
}

// Add default functionality here for any IAtomLocalMessageInterface functions that are not pure virtual.

void IAtomLocalMessageInterface::RecieveLocalMessage_Implementation(TSubclassOf<class UAtomLocalMessage> MessageClass, const int32 MessageIndex, 
	const FText& MessageText, class AAtomPlayerState* RelatedPlayerState_1, class AAtomPlayerState* RelatedPlayerState_2, UObject* OptionalObject)
{

}
