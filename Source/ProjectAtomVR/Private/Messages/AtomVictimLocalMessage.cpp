// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "Messages/AtomVictimLocalMessage.h"

UAtomVictimLocalMessage::UAtomVictimLocalMessage()
{
	KilledByMessage = NSLOCTEXT("AtomDeathMessage", "KilledByMessage", "Killed By {RelatedPlayerState_1}");
	SuicidedMessage = NSLOCTEXT("AtomDeathMessage", "SuicidedMessage", "Suicide");
	DiedMessage = NSLOCTEXT("AtomDeathMessage", "DiedMessage", "You Died");
}

FText UAtomVictimLocalMessage::GetRawText(const int32 MessageIndex, const FString& MessageString, 
	APlayerState* RelatedPlayerState_1, APlayerState* RelatedPlayerState_2, UObject* OptionalObject) const
{
	if (RelatedPlayerState_1 == nullptr)
	{
		return DiedMessage;
	}
	else if (RelatedPlayerState_1 != RelatedPlayerState_2)
	{
		return KilledByMessage;
	}
	else
	{
		return SuicidedMessage;
	}
}
