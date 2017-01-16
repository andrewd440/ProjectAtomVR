// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomOnlineSessionClient.h"

namespace
{
	constexpr int32 MaxSearchResults = 100;
}

UAtomOnlineSessionClient::UAtomOnlineSessionClient()
{

}

void UAtomOnlineSessionClient::RegisterOnlineDelegates()
{
	Super::RegisterOnlineDelegates();

	OnFindSessionsCompletedDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete);
}

bool UAtomOnlineSessionClient::StartOnlineSessionSearch()
{
	bool bOperationSuccessful = false;

	IOnlineSessionPtr SessionInt = GetSessionInt();
	if (SessionInt.IsValid() && (!SearchSettings.IsValid() || SearchSettings->SearchState != EOnlineAsyncTaskState::InProgress))
	{
		UE_LOG(LogAtomOnlineSession, Log, TEXT("Starting to find online sessions for %s"), *GetGameInstance()->GetFirstGamePlayer()->GetName());

		OnFindSessionsCompleteHandle = SessionInt->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompletedDelegate);

		SearchSettings = MakeShareable(new FOnlineSessionSearch());
		SearchSettings->MaxSearchResults = MaxSearchResults;

		bOperationSuccessful = SessionInt->FindSessions(*GetGameInstance()->GetFirstGamePlayer()->GetPreferredUniqueNetId(), SearchSettings.ToSharedRef());
	}

	return bOperationSuccessful;
}

bool UAtomOnlineSessionClient::StopOnlineSessionSearch()
{
	bool bOperationSuccessful = false;

	IOnlineSessionPtr SessionInt = GetSessionInt();
	if (SessionInt.IsValid())
	{
		SessionInt->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteHandle);

		bOperationSuccessful = SessionInt->CancelFindSessions();
	}

	return bOperationSuccessful;
}

TSharedPtr<FOnlineSessionSearch> UAtomOnlineSessionClient::GetSearchSettings() const
{
	return SearchSettings;
}

void UAtomOnlineSessionClient::OnFindSessionsComplete(bool bWasSuccessful)
{
	UE_LOG(LogAtomOnlineSession, Log, TEXT("Find sessions completed: %s, found %d sessions."), bWasSuccessful ? TEXT("Success") : TEXT("Failure"), bWasSuccessful ? SearchSettings->SearchResults.Num() : 0);

	IOnlineSessionPtr SessionInt = GetSessionInt();
	if (SessionInt.IsValid())
	{
		SessionInt->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteHandle);		
	}

	OnSessionSearchComplete.Broadcast(bWasSuccessful);
}
