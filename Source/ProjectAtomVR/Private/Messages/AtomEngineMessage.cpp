// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomEngineMessage.h"

UAtomEngineMessage::UAtomEngineMessage()	
{
	bIsConsoleMessage = true;

	NewPlayerMessage = NSLOCTEXT("AtomEngineMessage", "NewPlayerMessage", "New player joined the server.");
	EnteredMessage = NSLOCTEXT("AtomEngineMessage", "EnteredMessage", "{RelatedPlayerState_1} entered the game.");
	GlobalNameChange = NSLOCTEXT("AtomEngineMessage", "GlobalNameChange", "{RelatedPlayerState_1_Old} changed name to {RelatedPlayerState_1}.");
	LeftMessage = NSLOCTEXT("AtomEngineMessage", "LeftMessage", "{RelatedPlayerState_1} left the game.");
	MaxedOutMessage = NSLOCTEXT("AtomEngineMessage", "MaxedOutMessage", "{RelatedPlayerState_1} could not join full server.");
	NewSpecMessage = NSLOCTEXT("AtomEngineMessage", "NewSpecMessage", "New specator joined the server.");
	SpecEnteredMessage = NSLOCTEXT("AtomEngineMessage", "SpecEnteredMessage", "{RelatedPlayerState_1} entered the game as a spectator.");
}

FText UAtomEngineMessage::GetRawText(const int32 MessageIndex, const FString& MessageString, APlayerState* RelatedPlayerState_1, 
	APlayerState* RelatedPlayerState_2, UObject* OptionalObject) const
{
	//setup the local message string
	FText LocalMessage = FText::FromString(MessageString);

	//if we have a local message string, then don't use the localized string
	if (LocalMessage.IsEmpty())
	{
		const auto Index = static_cast<EAtomEngineMessageIndex>(MessageIndex);
		switch (Index)
		{
		case EAtomEngineMessageIndex::Entered:
			if (RelatedPlayerState_1 == nullptr)
			{
				LocalMessage = NewPlayerMessage;
			}
			else
			{
				LocalMessage = EnteredMessage;
			}
			break;
		case EAtomEngineMessageIndex::NameChanged:
			if (RelatedPlayerState_1 != nullptr)
			{
				LocalMessage = GlobalNameChange;
			}
			break;
		case EAtomEngineMessageIndex::Left:
			if (RelatedPlayerState_1 != nullptr)
			{
				LocalMessage = LeftMessage;
			}
			break;
		case EAtomEngineMessageIndex::MaxedOut:
			{
				LocalMessage = MaxedOutMessage;
			}
			break;
		case EAtomEngineMessageIndex::SpecEntered:
			if (RelatedPlayerState_1 == nullptr)
			{
				LocalMessage = NewSpecMessage;
			}
			else
			{
				LocalMessage = SpecEnteredMessage;
			}
			break;
		}
	}

	return LocalMessage;
}

void UAtomEngineMessage::GetRawTextArgs(FFormatNamedArguments& TextArgs, APlayerState* RelatedPlayerState_1, 
	APlayerState* RelatedPlayerState_2, UObject* OptionalObject) const
{
	Super::GetRawTextArgs(TextArgs, RelatedPlayerState_1, RelatedPlayerState_2, OptionalObject);

	TextArgs.Add(TEXT("RelatedPlayerState_1_Old"), RelatedPlayerState_1 != nullptr ?
		FText::FromString(RelatedPlayerState_1->OldName) : FText::GetEmpty());
}

