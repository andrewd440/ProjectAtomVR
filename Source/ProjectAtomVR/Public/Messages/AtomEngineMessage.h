// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Messages/AtomLocalMessage.h"
#include "AtomEngineMessage.generated.h"

UENUM(BlueprintType)
enum class EAtomEngineMessageIndex : uint8
{
	Entered = 1,
	NameChanged = 2,
	Left = 4,
	MaxedOut = 7,
	SpecEntered = 16
};

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UAtomEngineMessage : public UAtomLocalMessage
{
	GENERATED_BODY()

public:
	UAtomEngineMessage();
	
	/** Message displayed in message dialog when player pawn fails to spawn because no playerstart was available. */
	UPROPERTY()
	FText FailedPlaceMessage;

	/** Message when player join attempt is refused because the server is at capacity. */
	UPROPERTY()
	FText MaxedOutMessage;

	/** Message when a new player enters the game. */
	UPROPERTY()
	FText EnteredMessage;

	/** Message when a player leaves the game. */
	UPROPERTY()
	FText LeftMessage;

	/** Message when a player changes his name. */
	UPROPERTY()
	FText GlobalNameChange;

	/** Message when a new spectator enters the server (if spectator has a player name). */
	UPROPERTY()
	FText SpecEnteredMessage;

	/** Message when a new player enters the server (if player is unnamed). */
	UPROPERTY()
	FText NewPlayerMessage;

	/** Message when a new spectator enters the server (if spectator is unnamed). */
	UPROPERTY()
	FText NewSpecMessage;

protected:
	virtual FText GetRawText(const int32 MessageIndex, const FString& MessageString, APlayerState* RelatedPlayerState_1, 
		APlayerState* RelatedPlayerState_2, UObject* OptionalObject) const override;

	virtual void GetRawTextArgs(FFormatNamedArguments& TextArgs, APlayerState* RelatedPlayerState_1, APlayerState* RelatedPlayerState_2, UObject* OptionalObject) const override;

};
