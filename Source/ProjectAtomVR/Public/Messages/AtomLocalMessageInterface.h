// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "AtomLocalMessageInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UAtomLocalMessageInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

/**
 * 
 */
class PROJECTATOMVR_API IAtomLocalMessageInterface
{
	GENERATED_IINTERFACE_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, Category = AtomHUDLocalMessageWidget)
	void RecieveLocalMessage(TSubclassOf<class UAtomLocalMessage> MessageClass, const int32 MessageIndex, const FText& MessageText,
		class AAtomPlayerState* RelatedPlayerState_1, class AAtomPlayerState* RelatedPlayerState_2, UObject* OptionalObject);
	
};
