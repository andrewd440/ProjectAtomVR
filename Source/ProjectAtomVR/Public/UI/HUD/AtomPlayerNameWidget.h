// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "AtomPlayerNameWidget.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class PROJECTATOMVR_API UAtomPlayerNameWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent)
	void SetPlayerTalking(bool bIsTalking);

	UFUNCTION(BlueprintImplementableEvent)
	void SetPlayer(class AAtomPlayerState* Player);

	UFUNCTION(BlueprintImplementableEvent)
	void OnPlayerChangedTeams();
};
