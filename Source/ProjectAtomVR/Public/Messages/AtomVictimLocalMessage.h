// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Messages/AtomDeathLocalMessage.h"
#include "AtomVictimLocalMessage.generated.h"

/**
 * Message type used by when the active player dies.
 */
UCLASS()
class PROJECTATOMVR_API UAtomVictimLocalMessage : public UAtomLocalMessage
{
	GENERATED_BODY()
	
public:
	UAtomVictimLocalMessage();

	/** UAtomLocalMessage Interface Begin */
protected:
	virtual FText GetRawText(const int32 MessageIndex, const FString& MessageString, APlayerState* RelatedPlayerState_1, 
		APlayerState* RelatedPlayerState_2, UObject* OptionalObject) const override;
	/** UAtomLocalMessage Interface End */

protected:
	UPROPERTY(EditDefaultsOnly, Category = Message)
	FText KilledByMessage;

	UPROPERTY(EditDefaultsOnly, Category = Message)
	FText SuicidedMessage;

	UPROPERTY(EditDefaultsOnly, Category = Message)
	FText DiedMessage;
};
