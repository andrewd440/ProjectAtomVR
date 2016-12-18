// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomGameInstance.h"

#include "../../OnlineSubsystemUtils/Source/OnlineSubsystemUtils/Classes/OnlineSessionClient.h"
#include "OnlineSessionInterface.h"
#include "../../OnlineSubsystemUtils/Source/OnlineSubsystemUtils/Public/OnlineSubsystemUtils.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineSessionSettings.h"

DEFINE_LOG_CATEGORY_STATIC(LogAtomGameInstance, Log, All);

namespace
{
	constexpr int32 MaxSessionConnections = 10;
}

UAtomGameInstance::UAtomGameInstance()
{
	OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UAtomGameInstance::OnCreateSessionComplete);
	OnFindSessionsCompletedDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &UAtomGameInstance::OnSearchSessionsComplete);
	OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UAtomGameInstance::OnJoinSessionComplete);
}

TSubclassOf<UOnlineSession> UAtomGameInstance::GetOnlineSessionClass()
{
	return UOnlineSessionClient::StaticClass();
}

bool UAtomGameInstance::StartSessionSearch()
{
	return true;
}

bool UAtomGameInstance::EndSessionSearch()
{
	return true;
}

void UAtomGameInstance::OnSearchSessionsComplete(bool bWasSuccessful)
{

}

bool UAtomGameInstance::CreateSession()
{
	UE_LOG(LogAtomGameInstance, Log, TEXT("Creating online session for %s"), *GetFirstGamePlayer()->GetName());

	IOnlineSessionPtr SessionInt = Online::GetSessionInterface(GetWorld());
	if (SessionInt.IsValid())
	{
		// Create lobby session
		FOnlineSessionSettings Settings;
		Settings.bAllowInvites = true;
		Settings.bAllowJoinInProgress = true;
		Settings.NumPublicConnections = MaxSessionConnections;

		Settings.Set(SETTING_MAPNAME, OnlineLobbyMap.ToString(), EOnlineDataAdvertisementType::ViaOnlineService);
		Settings.Set(SETTING_GAMEMODE, OnlineLobbyGameMode.ToString(), EOnlineDataAdvertisementType::ViaOnlineService);

		OnCreateSessionCompleteHandle = SessionInt->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);
		return SessionInt->CreateSession(*GetFirstGamePlayer()->GetPreferredUniqueNetId(), GameSessionName, Settings);
	}
	else
	{
		UE_LOG(LogAtomGameInstance, Warning, TEXT("CreateSession failed. Could not find a valid online session interface."));
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
	return true;
}

bool UAtomGameInstance::JoinSession(ULocalPlayer* LocalPlayer, int32 SessionIndexInSearchResults)
{
	return true;
}

void UAtomGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{

}
