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
	enum class EType : uint8
	{
		RoundStart,
		MinuteWarning,
		RoundEnd
	};

	static int32 ConstructMessageIndex(const EType Type, const int32 Count);
	static void ExtractMessageIndex(const int32 Index, EType& Type, int32& Count);	

public:
	UAtomCountdownMessage();

public:
	UPROPERTY(EditDefaultsOnly, Category = CountdownMessage)
	FText RoundStartPrefix;
	
	UPROPERTY(EditDefaultsOnly, Category = CountdownMessage)
	FText RoundEndPrefix;

	UPROPERTY(EditDefaultsOnly, Category = CountdownMessage)
	FText CountdownMessage;

	UPROPERTY(EditDefaultsOnly, Category = CountdownMessage)
	FText MinuteWarningMessage;

	/** UAtomLocalMessage Interface Begin */
public:
	virtual int32 GetStatusMessageDuration(const int32 MessageIndex) const override;
	virtual bool IsStatusMessage(const int32 MessageIndex) const override;
protected:
	virtual FText GetRawText(const int32 MessageIndex, const FString& MessageString, APlayerState* RelatedPlayerState_1, 
		APlayerState* RelatedPlayerState_2, UObject* OptionalObject) const override;
	virtual void GetRawTextArgs(FFormatNamedArguments& TextArgs, const int32 MessageIndex, APlayerState* RelatedPlayerState_1, 
		APlayerState* RelatedPlayerState_2, UObject* OptionalObject) const override;
	/** UAtomLocalMessage Interface End */
};
