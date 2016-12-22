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

	/**
	* Creates an online session and loads the online lobby map.
	*/
	UFUNCTION(BlueprintCallable, Category = AtomGameInstance)
	virtual bool CreateSession();

	class UAtomPlaylistManager* GetPlaylistManager() const;

	FName GetLobbyMap() const { return OnlineLobbyMap; }
	FName GetLobbyGameMode() const { return OnlineLobbyGameMode; }

	/** UGameInstance Interface Begin */
	virtual TSubclassOf<UOnlineSession> GetOnlineSessionClass() override;
	virtual bool JoinSession(ULocalPlayer* LocalPlayer, const FOnlineSessionSearchResult& SearchResult) override;
	virtual bool JoinSession(ULocalPlayer* LocalPlayer, int32 SessionIndexInSearchResults) override;
	/** UGameInstance Interface End */

protected:
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AtomGameInstance)
	FName OnlineLobbyMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AtomGameInstance)
	FName OnlineLobbyGameMode;

	UPROPERTY(EditAnywhere, Instanced, Category = AtomGameInstance)
	class UAtomPlaylistManager* PlaylistManager;

	// Handle for network operations
	FDelegateHandle OnCreateSessionCompleteHandle;
	FDelegateHandle OnJoinSessionCompleteHandle;

	// Delegates for network operations
	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;
};
