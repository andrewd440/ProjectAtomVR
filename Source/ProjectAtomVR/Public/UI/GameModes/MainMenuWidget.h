// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UMainMenuWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	UFUNCTION(BlueprintCallable, Category = MainMenuWidget)
	bool OnFindMatch();
	
	UFUNCTION(BlueprintImplementableEvent, Category = MainMenuWidget)
	void OnFindMatchFinished();

	void OnOnlineSessionSearchComplete(bool bWasSuccessful);

protected:
	UPROPERTY(BlueprintReadOnly, Category = MainMenuWidget)
	uint32 bIsSearching : 1;

	FDelegateHandle OnOnlineSessionSearchCompleteDelegateHandle;
};
