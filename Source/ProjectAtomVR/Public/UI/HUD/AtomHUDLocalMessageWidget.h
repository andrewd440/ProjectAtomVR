// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "AtomHUDLocalMessageWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UAtomHUDLocalMessageWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent, Category = AtomHUDLocalMessageWidget)
	void InitializeWithMessage(TSubclassOf<class UAtomLocalMessage> MessageClass, const int32 MessageIndex, const FText& MessageText, 
		class AAtomPlayerState* RelatedPlayerState_1, class AAtomPlayerState* RelatedPlayerState_2, UObject* OptionalObject);
};
