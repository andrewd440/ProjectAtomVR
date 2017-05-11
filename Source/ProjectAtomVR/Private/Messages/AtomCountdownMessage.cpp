// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomCountdownMessage.h"

int32 UAtomCountdownMessage::ConstructMessageIndex(const EType Type, const int32 Count)
{
	return static_cast<uint8>(Type) | (Count << 8);
}

void UAtomCountdownMessage::ExtractMessageIndex(const int32 Index, EType& Type, int32& Count)
{
	Type = static_cast<EType>(Index & 0xFF);
	Count = (Index >> 8);
}

UAtomCountdownMessage::UAtomCountdownMessage()
{
	DisplayTime = 5.f;

	CountdownMessage = NSLOCTEXT("AtomCountdownMessage", "CountdownMessage", "{Count}");

	RoundStartPrefix = NSLOCTEXT("AtomCountdownMessage", "RoundStartPrefix", "Round will begin in");

	RoundEndPrefix = NSLOCTEXT("AtomCountdownMessage", "RoundEndPrefix", "Round will end in");

	MinuteWarningMessage = NSLOCTEXT("AtomCountdownMessage", "MinuteWarningMessage", "One minute remaining");
}

FText UAtomCountdownMessage::GetRawText(const int32 MessageIndex, const FString& MessageString, APlayerState* RelatedPlayerState_1, 
	APlayerState* RelatedPlayerState_2, UObject* OptionalObject) const
{	
	static const FTextFormat CountdownFormat = FTextFormat{ NSLOCTEXT("AtomCountdownMessage", "CountdownFormat", "{0} {1}") };

	EType Type; int32 Count;
	ExtractMessageIndex(MessageIndex, Type, Count);

	FText Text;
	switch (Type)
	{
	case EType::RoundStart:
		Text = FText::FormatOrdered(CountdownFormat, RoundStartPrefix, Count);
		break;
	case EType::MinuteWarning:
		Text = MinuteWarningMessage;
		break;
	case EType::RoundEnd:
		Text = FText::FormatOrdered(CountdownFormat, RoundEndPrefix, Count);
		break;
	default:
		check(false && "Unhandled type.");
	}

	return Text;
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

int32 UAtomCountdownMessage::GetStatusMessageDuration(const int32 MessageIndex) const
{
	EType Type; int32 Count;
	ExtractMessageIndex(MessageIndex, Type, Count);

	if (Type == EType::MinuteWarning)
	{
		return DisplayTime;
	}
	else
	{
		return 1.05f; // Leave time for next count
	}
}