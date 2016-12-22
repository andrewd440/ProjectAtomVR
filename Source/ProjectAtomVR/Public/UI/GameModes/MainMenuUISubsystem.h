// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "UI/GameModeUISubsystem.h"
#include "MainMenuUISubsystem.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UMainMenuUISubsystem : public UGameModeUISubsystem
{
	GENERATED_BODY()
	
public:
	/** UGameModeUISubsystem Interface Begin */
	virtual void InitializeSystem(AAtomUISystem* Owner, AAtomBaseGameMode* GameModeCDO) override;
	virtual void Destroy() override;
	/** UGameModeUISubsystem Interface End */

protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = MainMenuUI)
	TSubclassOf<class UUserWidget> MainMenuWidget;

	UPROPERTY()
	AAtomFloatingUI* MainMenuUI = nullptr;
};
