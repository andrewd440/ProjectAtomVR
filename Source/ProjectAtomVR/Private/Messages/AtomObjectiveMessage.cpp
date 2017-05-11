// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomObjectiveMessage.h"

int32 UAtomObjectiveMessage::ConstructMessageIndex(const EType Type, const int32 StatusDuration /*= 0*/)
{
	return static_cast<int32>(Type) | (StatusDuration << 8);
}

UAtomObjectiveMessage::UAtomObjectiveMessage()
{
	DisplayTime = 10.f;

	WaitingForObjectiveMessage = NSLOCTEXT("AtomObjectiveMessage", "WaitingForObjective", "Waiting For Objective.");

	CapturingObjectiveMessage = NSLOCTEXT("AtomObjectiveMessage", "CapturingObjective", "Capturing Objective");

	LosingObjectiveMessage = NSLOCTEXT("AtomObjectiveMessage", "LosingObjective", "Losing Objective");

	NewObjectiveMessage = NSLOCTEXT("AtomObjectiveMessage", "NewObjective", "New Objective Spawned.");	

	LostObjectiveMessage = NSLOCTEXT("AtomObjectiveMessage", "LostObjective", "Objective Lost");

	CapturedObjectiveMessage = NSLOCTEXT("AtomObjectiveMessage", "CapturedObjective", "Objective Captured");
}

bool UAtomObjectiveMessage::IsStatusMessage(const int32 MessageIndex) const
{
	return (MessageIndex & 0xFF) < 3;
}

int32 UAtomObjectiveMessage::GetStatusMessageDuration(const int32 MessageIndex) const
{
	return MessageIndex >> 8;
}

FText UAtomObjectiveMessage::GetRawText(const int32 MessageIndex, const FString& MessageString, APlayerState* RelatedPlayerState_1, APlayerState* RelatedPlayerState_2, UObject* OptionalObject) const
{
	const EType Type = static_cast<EType>(MessageIndex & 0xFF);
	switch (Type)
	{
	case EType::WaitingForObjective:
		return WaitingForObjectiveMessage;
	case EType::CapturingObjective:
		return CapturingObjectiveMessage;
	case EType::LosingObjective:
		return LosingObjectiveMessage;
	case EType::NewObjective:
		return NewObjectiveMessage;
	case EType::LostObjective:
		return LostObjectiveMessage;
	case EType::CapturedObjective:
		return CapturedObjectiveMessage;
	default:
		check(false && "Invalid message index.");
	}

	return FText::GetEmpty();
}
