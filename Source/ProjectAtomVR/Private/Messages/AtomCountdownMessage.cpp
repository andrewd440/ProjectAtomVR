// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomCountdownMessage.h"

UAtomCountdownMessage::UAtomCountdownMessage()
{
	DisplayTime = 1.05f; // Leave time for next count

	CountdownMessage = NSLOCTEXT("AtomCountdownMessage", "CountdownMessage", "{Count}...");
	CountdownEnd = NSLOCTEXT("AtomCountdownMessage", "CountdownEnd", "Ready!");
}

FText UAtomCountdownMessage::GetRawText(const int32 MessageIndex, const FString& MessageString, APlayerState* RelatedPlayerState_1, 
	APlayerState* RelatedPlayerState_2, UObject* OptionalObject) const
{
	return (MessageIndex > 0) ? CountdownMessage : CountdownEnd;
}

void UAtomCountdownMessage::GetRawTextArgs(FFormatNamedArguments& TextArgs, const int32 MessageIndex, APlayerState* RelatedPlayerState_1, 
	APlayerState* RelatedPlayerState_2, UObject* OptionalObject) const
{
	Super::GetRawTextArgs(TextArgs, MessageIndex, RelatedPlayerState_1, RelatedPlayerState_2, OptionalObject);
	TextArgs.Add(TEXT("Count"), FText::AsNumber(MessageIndex));
}

bool UAtomCountdownMessage::IsStatusMessage(const int32 MessageIndex) const
{
	return true;
}
