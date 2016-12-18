// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Engine/GameInstance.h"
#include "OnlineSessionInterface.h"
#include "AtomGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UAtomGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UAtomGameInstance();

	virtual bool StartSessionSearch();
	virtual bool EndSessionSearch();

	/**
	* Creates an online session and loads the online lobby map.
	*/
	UFUNCTION(BlueprintCallable, Category = AtomGameInstance)
	virtual bool CreateSession();

	/** UGameInstance Interface Begin */
	virtual TSubclassOf<UOnlineSession> GetOnlineSessionClass() override;
	virtual bool JoinSession(ULocalPlayer* LocalPlayer, const FOnlineSessionSearchResult& SearchResult) override;
	virtual bool JoinSession(ULocalPlayer* LocalPlayer, int32 SessionIndexInSearchResults) override;
	/** UGameInstance Interface End */

protected:
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	void OnSearchSessionsComplete(bool bWasSuccessful);

	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AtomGameInstance)
	FName OnlineLobbyMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AtomGameInstance)
	FName OnlineLobbyGameMode;

	// Handle for network operations
	FDelegateHandle OnCreateSessionCompleteHandle;
	FDelegateHandle OnFindSessionsCompleteHandle;
	FDelegateHandle OnJoinSessionCompleteHandle;

	// Delegates for network operations
	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
	FOnFindSessionsCompleteDelegate OnFindSessionsCompletedDelegate;
	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;
};
