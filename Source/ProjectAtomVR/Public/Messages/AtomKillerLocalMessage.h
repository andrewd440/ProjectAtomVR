// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Messages/AtomLocalMessage.h"
#include "AtomKillerLocalMessage.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UAtomKillerLocalMessage : public UAtomLocalMessage
{
	GENERATED_BODY()
		
public:
	UAtomKillerLocalMessage();

	/** UAtomLocalMessage Interface Begin */
protected:
	virtual FText GetRawText(const int32 MessageIndex, const FString& MessageString, APlayerState* RelatedPlayerState_1,
		APlayerState* RelatedPlayerState_2, UObject* OptionalObject) const override;
	/** UAtomLocalMessage Interface End */

protected:
	UPROPERTY(EditDefaultsOnly, Category = Message)
	FText KillMessage;	
};
