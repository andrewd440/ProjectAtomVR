// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "OnlineSessionClient.h"
#include "AtomOnlineSessionClient.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UAtomOnlineSessionClient : public UOnlineSessionClient
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnSessionSearchComplete, bool /*bWasSuccessful*/);
	FOnSessionSearchComplete OnSessionSearchComplete;

public:
	UAtomOnlineSessionClient();

	/**
	* Starts searching for online sessions.
	* 
	* @returns True if the operation start was successful. The search operation is async and 
	* is not completed until the OnOnlineSessionSearchCompleted delegate is called.
	*/
	bool StartOnlineSessionSearch();

	/**
	* Stops any active search for online sessions.
	* 
	* @returns True if a search was active and has been stopped.
	*/
	bool StopOnlineSessionSearch();

	/**
	* Gets the search settings used for the last search.
	*/
	TSharedPtr<FOnlineSessionSearch> GetSearchSettings() const;

	/** UOnlineSessionClient Interface Begin */
	virtual void RegisterOnlineDelegates() override;
	/** UOnlineSessionClient Interface End */	

protected:
	void OnFindSessionsComplete(bool bWasSuccessful);

protected:
	TSharedPtr<FOnlineSessionSearch> SearchSettings;

	FOnFindSessionsCompleteDelegate OnFindSessionsCompletedDelegate;

	FDelegateHandle OnFindSessionsCompleteHandle;	
};
