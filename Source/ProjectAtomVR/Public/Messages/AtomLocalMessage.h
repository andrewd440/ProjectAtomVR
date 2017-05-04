// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/LocalMessage.h"
#include "AtomLocalMessage.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class PROJECTATOMVR_API UAtomLocalMessage : public ULocalMessage
{
	GENERATED_BODY()
		
public:
	UAtomLocalMessage();

	virtual void ClientReceive(const FClientReceiveData& ClientData) const override;

	FText GetFormattedText(const int32 MessageIndex, const FString& MessageString, APlayerState* RelatedPlayerState_1,
		APlayerState* RelatedPlayerState_2, UObject* OptionalObject) const;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = AtomLocalMessage)
	void OnClientReceive(const FClientReceiveData& ClientData) const;

	virtual FText GetRawText(const int32 MessageIndex, const FString& MessageString, APlayerState* RelatedPlayerState_1,
		APlayerState* RelatedPlayerState_2, UObject* OptionalObject) const;

	virtual void GetRawTextArgs(FFormatNamedArguments& TextArgs, APlayerState* RelatedPlayerState_1, 
		APlayerState* RelatedPlayerState_2, UObject* OptionalObject) const;

	/** Returns true if this message should be displayed in the status HUD UI */
	virtual bool IsStatusMessage(const int32 MessageIndex) const;

public:
	/** Seconds to display the message */
	UPROPERTY(EditDefaultsOnly, Category = AtomLocalMessage)
	float DisplayTime = 2.f;

	/** Widget to display on the HUD for this message. */
	UPROPERTY(EditDefaultsOnly, Category = AtomLocalMessage)
	TSubclassOf<class UUserWidget> HUDWidget;

	/** Location offset to spawn the HUD widget for this message in relation to the camera. */
	UPROPERTY(EditDefaultsOnly, Category = AtomLocalMessage)
	FVector HUDWidgetLocationOffset;

	/** True if this message should be output to the console */
	UPROPERTY(EditDefaultsOnly, Category = AtomLocalMessage)
	uint32 bIsConsoleMessage : 1;
};
