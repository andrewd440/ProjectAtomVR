// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once
#include "OnlineSessionSettings.h"

/** Setting describing if the game is in the lobby (value is bool) */
#define SETTING_ISLOBBY FName(TEXT("ISLOBBY"))

class PROJECTATOMVR_API FAtomOnlineSessionSettings : public FOnlineSessionSettings
{
public:
	FAtomOnlineSessionSettings(bool bIsLobby)
	{
		Set(SETTING_ISLOBBY, bIsLobby, EOnlineDataAdvertisementType::ViaOnlineService);
	}

	~FAtomOnlineSessionSettings() {}
};

/**
 * 
 */
class PROJECTATOMVR_API FAtomOnlineSessionSearch : public FOnlineSessionSearch
{
public:
	FAtomOnlineSessionSearch() {}
	~FAtomOnlineSessionSearch() {}

	virtual void SortSearchResults() override;
};
