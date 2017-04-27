// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Messages/AtomLocalMessage.h"
#include "AtomDeathLocalMessage.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UAtomDeathLocalMessage : public UAtomLocalMessage
{
	GENERATED_BODY()
		
public:
	UAtomDeathLocalMessage();

	/** UAtomLocalMessage Interface Begin */
public:
	virtual void ClientReceive(const FClientReceiveData& ClientData) const override;
protected:
	virtual FText GetRawText(const int32 MessageIndex, const FString& MessageString, APlayerState* RelatedPlayerState_1,
		APlayerState* RelatedPlayerState_2, UObject* OptionalObject) const override;
	/** UAtomLocalMessage Interface End */

protected:
	UPROPERTY(EditDefaultsOnly, Category = Message)
	TSubclassOf<class UAtomKillerLocalMessage> KillerMessageClass;

	UPROPERTY(EditDefaultsOnly, Category = Message)
	TSubclassOf<class UAtomVictimLocalMessage> VictimMessageClass;

	UPROPERTY(EditDefaultsOnly, Category = Message)
	FText GenericKillMessage;

	UPROPERTY(EditDefaultsOnly, Category = Message)
	FText GenericSuicideMessage;

	UPROPERTY(EditDefaultsOnly, Category = Message)
	FText GenericDeathMessage;
};
