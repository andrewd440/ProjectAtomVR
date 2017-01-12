// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/GameUserSettings.h"
#include "AtomPlayerSettings.h"
#include "AtomGameUserSettings.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class PROJECTATOMVR_API UAtomGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()
	
public:
	FAtomPlayerSettings& GetPlayerSettings();

	/** UGameUserSettings Interface Begin */
	virtual void LoadSettings(bool bForceReload = false) override;
	virtual void SaveSettings() override;
	/** UGameUserSettings Interface End */

protected:
	virtual void LoadPlayerSettings();
	virtual void SavePlayerSettings();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AtomGameUserSettings)
	FAtomPlayerSettings PlayerSettings;
};