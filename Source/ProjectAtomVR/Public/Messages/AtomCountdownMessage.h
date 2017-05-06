// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Messages/AtomLocalMessage.h"
#include "AtomCountdownMessage.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UAtomCountdownMessage : public UAtomLocalMessage
{
	GENERATED_BODY()
	
public:	
	UAtomCountdownMessage();

public:
	FText CountdownMessage;
	FText CountdownEnd;

	/** UAtomLocalMessage Interface Begin */
public:
	virtual bool IsStatusMessage(const int32 MessageIndex) const override;
protected:
	virtual FText GetRawText(const int32 MessageIndex, const FString& MessageString, APlayerState* RelatedPlayerState_1, 
		APlayerState* RelatedPlayerState_2, UObject* OptionalObject) const override;
	virtual void GetRawTextArgs(FFormatNamedArguments& TextArgs, const int32 MessageIndex, APlayerState* RelatedPlayerState_1, 
		APlayerState* RelatedPlayerState_2, UObject* OptionalObject) const override;
	/** UAtomLocalMessage Interface End */
};
