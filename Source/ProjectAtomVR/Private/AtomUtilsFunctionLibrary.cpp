// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomUtilsFunctionLibrary.h"


void UAtomUtilsFunctionLibrary::TrimClassName(FString& Name)
{
	Name.RemoveFromStart(TEXT("BP_"), ESearchCase::IgnoreCase);
	Name.RemoveFromStart(TEXT("UI_"), ESearchCase::IgnoreCase);

	Name.RemoveFromEnd(TEXT("_C"), ESearchCase::CaseSensitive);

	Name.ReplaceInline(TEXT("_"), TEXT(" "), ESearchCase::IgnoreCase);
}
