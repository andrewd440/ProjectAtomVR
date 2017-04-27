// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "Messages/AtomKillerLocalMessage.h"

UAtomKillerLocalMessage::UAtomKillerLocalMessage()
{
	KillMessage = NSLOCTEXT("AtomKillerLocalMessage", "KillMessage", "You Killed {RelatedPlayerState_2}");
}

FText UAtomKillerLocalMessage::GetRawText(const int32 MessageIndex, const FString& MessageString, 
	APlayerState* RelatedPlayerState_1, APlayerState* RelatedPlayerState_2, UObject* OptionalObject) const
{
	return KillMessage;
}
