// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "AtomUtilsFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UAtomUtilsFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	/**
	* Trims prefixes (BP_, UI_, etc.) and suffixes (_C) from a class name.
	*/
	UFUNCTION(BlueprintCallable, Category = AtomUtils)
	static void TrimClassName(FString& Name);
};
