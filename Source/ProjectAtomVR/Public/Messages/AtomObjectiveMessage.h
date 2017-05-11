// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Messages/AtomLocalMessage.h"
#include "AtomObjectiveMessage.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UAtomObjectiveMessage : public UAtomLocalMessage
{
	GENERATED_BODY()
	
public:
	enum class EType : uint8
	{
		WaitingForObjective,
		CapturingObjective,
		LosingObjective,
		NewObjective,
		LostObjective,
		CapturedObjective
	};	

public:
	static int32 ConstructMessageIndex(const EType Type, const int32 StatusDuration = 0);

public:

	UAtomObjectiveMessage();

	/** UAtomLocalMessage Interface Begin */
public:
	virtual int32 GetStatusMessageDuration(const int32 MessageIndex) const override;
	virtual bool IsStatusMessage(const int32 MessageIndex) const override;
protected:
	virtual FText GetRawText(const int32 MessageIndex, const FString& MessageString, APlayerState* RelatedPlayerState_1, 
		APlayerState* RelatedPlayerState_2, UObject* OptionalObject) const override;
	/** UAtomLocalMessage Interface End */

public:

	UPROPERTY(EditDefaultsOnly, Category = ObjectiveMessage)
	FText WaitingForObjectiveMessage;
	
	UPROPERTY(EditDefaultsOnly, Category = ObjectiveMessage)
	FText CapturingObjectiveMessage;

	UPROPERTY(EditDefaultsOnly, Category = ObjectiveMessage)
	FText LosingObjectiveMessage;

	UPROPERTY(EditDefaultsOnly, Category = ObjectiveMessage)
	FText NewObjectiveMessage;

	UPROPERTY(EditDefaultsOnly, Category = ObjectiveMessage)
	FText LostObjectiveMessage;

	UPROPERTY(EditDefaultsOnly, Category = ObjectiveMessage)
	FText CapturedObjectiveMessage;
};
