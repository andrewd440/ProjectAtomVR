// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "Messages/AtomLocalMessage.h"
#include "AtomPlayerController.h"
#include "UI/HUD/VRHUD.h"

void UAtomLocalMessage::ClientReceive(const FClientReceiveData& ClientData) const
{
	if (auto AtomPC = Cast<AAtomPlayerController>(ClientData.LocalPC))
	{
		if (auto VRHUD = AtomPC->GetVRHUD())
		{
			const FText MessageText = GetFormattedText(ClientData.MessageIndex, ClientData.MessageString, ClientData.RelatedPlayerState_1,
				ClientData.RelatedPlayerState_2, ClientData.OptionalObject);

			VRHUD->ReceiveLocalMessage(GetClass(), ClientData.MessageIndex, MessageText, ClientData.RelatedPlayerState_1,
				ClientData.RelatedPlayerState_2, ClientData.OptionalObject);
		}
	}
}

FText UAtomLocalMessage::GetFormattedText(const int32 MessageIndex, const FString& MessageString, APlayerState* RelatedPlayerState_1, 
	APlayerState* RelatedPlayerState_2, UObject* OptionalObject) const
{
	FFormatNamedArguments TextArgs;
	GetRawTextArgs(TextArgs, RelatedPlayerState_1, RelatedPlayerState_2, OptionalObject);
	return FText::Format(GetRawText(MessageIndex, MessageString, RelatedPlayerState_1, RelatedPlayerState_2, OptionalObject), TextArgs);
}

FText UAtomLocalMessage::GetRawText(const int32 MessageIndex, const FString& MessageString, APlayerState* RelatedPlayerState_1, 
	APlayerState* RelatedPlayerState_2, UObject* OptionalObject) const
{
	return FText::GetEmpty();
}

void UAtomLocalMessage::GetRawTextArgs(FFormatNamedArguments& TextArgs, APlayerState* RelatedPlayerState_1,
	APlayerState* RelatedPlayerState_2, UObject* OptionalObject) const
{
	TextArgs.Add(TEXT("RelatedPlayerState_1"), RelatedPlayerState_1 != nullptr ? FText::FromString(RelatedPlayerState_1->PlayerName) : FText::GetEmpty());
	TextArgs.Add(TEXT("RelatedPlayerState_2"), RelatedPlayerState_2 != nullptr ? FText::FromString(RelatedPlayerState_2->PlayerName) : FText::GetEmpty());
}
