// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomGameInstance.h"

#include "OnlineSessionInterface.h"
#include "../../OnlineSubsystemUtils/Source/OnlineSubsystemUtils/Public/OnlineSubsystemUtils.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineSessionSettings.h"
#include "Online/AtomOnlineSessionClient.h"

DEFINE_LOG_CATEGORY_STATIC(LogAtomGameInstance, Log, All);

namespace
{
	constexpr int32 MaxSessionConnections = 10;
}

UAtomGameInstance::UAtomGameInstance()
{
	OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UAtomGameInstance::OnCreateSessionComplete);
	OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UAtomGameInstance::OnJoinSessionComplete);
}

TSubclassOf<UOnlineSession> UAtomGameInstance::GetOnlineSessionClass()
{
	return UAtomOnlineSessionClient::StaticClass();
}

bool UAtomGameInstance::CreateSession()
{
	UE_LOG(LogAtomOnlineSession, Log, TEXT("Creating online session for %s"), *GetFirstGamePlayer()->GetName());

	IOnlineSessionPtr SessionInt = Online::GetSessionInterface(GetWorld());
	if (SessionInt.IsValid())
	{
		// Create lobby session
		FOnlineSessionSettings Settings;
		Settings.bAllowInvites = true;
		Settings.bAllowJoinInProgress = true;
		Settings.NumPublicConnections = MaxSessionConnections;
		Settings.bAllowJoinViaPresence = true;
		Settings.bShouldAdvertise = true;

		Settings.Set(SETTING_MAPNAME, OnlineLobbyMap.ToString(), EOnlineDataAdvertisementType::ViaOnlineService);
		Settings.Set(SETTING_GAMEMODE, OnlineLobbyGameMode.ToString(), EOnlineDataAdvertisementType::ViaOnlineService);

		OnCreateSessionCompleteHandle = SessionInt->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);
		return SessionInt->CreateSession(*GetFirstGamePlayer()->GetPreferredUniqueNetId(), GameSessionName, Settings);
	}
	else
	{
		UE_LOG(LogAtomOnlineSession, Warning, TEXT("CreateSession failed. Could not find a valid online session interface."));
		return false;
	}
}

void UAtomGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSessionPtr SessionInt = Online::GetSessionInterface(GetWorld());

	if(SessionInt.IsValid())
	{
		SessionInt->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteHandle);

		if (bWasSuccessful)
		{
			const FString URLString = FString::Printf(TEXT("%s?listen?game=%s"), *OnlineLobbyMap.ToString(), *OnlineLobbyGameMode.ToString());

			// Travel to the online lobby
			GetWorld()->ServerTravel(URLString);
		}
		else
		{
			// #AtomTodo Send notification of error
		}
	}
}

bool UAtomGameInstance::JoinSession(ULocalPlayer* LocalPlayer, const FOnlineSessionSearchResult& SearchResult)
{
	bool bOperationSuccessful = false;

	if (SearchResult.IsValid())
	{
		UE_LOG(LogAtomOnlineSession, Log, TEXT("Starting to join online session by %s for %s"), *SearchResult.Session.OwningUserName, *LocalPlayer->GetName());

		IOnlineSessionPtr SessionInt = Online::GetSessionInterface(GetWorld());
		if (SessionInt.IsValid())
		{
			check(!SessionInt->IsPlayerInSession(GameSessionName, *LocalPlayer->GetPreferredUniqueNetId())); // Should not already be in a session

			OnJoinSessionCompleteHandle = SessionInt->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);

			SessionInt->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), GameSessionName, SearchResult);
		}
	}
	else
	{
		UE_LOG(LogAtomOnlineSession, Warning, TEXT("%s attempted to join online session using invalid search result."), *LocalPlayer->GetName());
	}

	return bOperationSuccessful;
}

bool UAtomGameInstance::JoinSession(ULocalPlayer* LocalPlayer, int32 SessionIndexInSearchResults)
{
	if (UAtomOnlineSessionClient* AtomOnlineSession = Cast<UAtomOnlineSessionClient>(GetOnlineSession()))
	{
		TSharedPtr<FOnlineSessionSearch> SearchSettings = AtomOnlineSession->GetSearchSettings();

		if (SearchSettings.IsValid() && SearchSettings->SearchResults.IsValidIndex(SessionIndexInSearchResults))
		{
			return JoinSession(LocalPlayer, SearchSettings->SearchResults[SessionIndexInSearchResults]);
		}
	}

	return false;
}

void UAtomGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSessionPtr SessionInt = Online::GetSessionInterface(GetWorld());
	if (SessionInt.IsValid())
	{
		SessionInt->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteHandle);
	}

	// #AtomTodo Handle other result types
	if (AGameModeBase* GameMode = GetWorld()->GetAuthGameMode())
	{
		if (GameMode->GameSession != nullptr)
		{
			switch (Result)
			{
				case EOnJoinSessionCompleteResult::Success:
				{
					ULocalPlayer* const LocalPlayer = GetFirstGamePlayer();
					GameMode->GameSession->TravelToSession(LocalPlayer->GetControllerId(), SessionName);
					break;
				}
			}
		}		
	}	
}
