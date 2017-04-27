// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "Messages/AtomDeathLocalMessage.h"
#include "Messages/AtomKillerLocalMessage.h"
#include "Messages/AtomVictimLocalMessage.h"

UAtomDeathLocalMessage::UAtomDeathLocalMessage()
{
	KillerMessageClass = UAtomKillerLocalMessage::StaticClass();
	VictimMessageClass = UAtomVictimLocalMessage::StaticClass();

	GenericKillMessage = NSLOCTEXT("AtomDeathMessage", "GenericKillMessage", "{RelatedPlayerState_1} Killed {RelatedPlayerState_2}");
	GenericSuicideMessage = NSLOCTEXT("AtomDeathMessage", "GenericSuicideMessage", "{RelatedPlayerState_1} Suicided");
	GenericDeathMessage = NSLOCTEXT("AtomDeathMessage", "GenericDeathMessage", "{RelatedPlayerState_2} Died");
}

FText UAtomDeathLocalMessage::GetRawText(const int32 MessageIndex, const FString& MessageString, APlayerState* RelatedPlayerState_1, APlayerState* RelatedPlayerState_2, UObject* OptionalObject) const
{
	check(RelatedPlayerState_1 || RelatedPlayerState_2);
	if (RelatedPlayerState_1 != RelatedPlayerState_2)
	{
		return GenericKillMessage;
	}
	else if (RelatedPlayerState_1 == RelatedPlayerState_2)
	{
		return GenericSuicideMessage;
	}
	else
	{
		return GenericDeathMessage;
	}
}

void UAtomDeathLocalMessage::ClientReceive(const FClientReceiveData& ClientData) const
{
	APlayerState* LocalPlayerState = ClientData.LocalPC->PlayerState;

	if (LocalPlayerState != nullptr && LocalPlayerState == ClientData.RelatedPlayerState_2 && VictimMessageClass != nullptr)
	{
		// Has been victimized. Switch to victim message.
		VictimMessageClass->GetDefaultObject<UAtomLocalMessage>()->ClientReceive(ClientData);
	}
	else if (LocalPlayerState != nullptr && LocalPlayerState == ClientData.RelatedPlayerState_1 && KillerMessageClass != nullptr)
	{
		// Is killer. Switch to kill message.
		KillerMessageClass->GetDefaultObject<UAtomLocalMessage>()->ClientReceive(ClientData);
	}
	else
	{
		// Not killer or victim. Show default death message.
		Super::ClientReceive(ClientData);
	}	
}
