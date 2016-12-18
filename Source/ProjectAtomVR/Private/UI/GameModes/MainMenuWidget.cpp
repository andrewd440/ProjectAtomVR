// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "MainMenuWidget.h"
#include "Online/AtomOnlineSessionClient.h"
#include "AtomGameInstance.h"


UMainMenuWidget::UMainMenuWidget(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: UUserWidget(ObjectInitializer)
{

}

bool UMainMenuWidget::OnFindMatch()
{
	bool bSuccess = false;

	UAtomGameInstance* GameInstance = Cast<UAtomGameInstance>(GetWorld()->GetGameInstance());
	UAtomOnlineSessionClient* OnlineSession = (GameInstance != nullptr) ? Cast<UAtomOnlineSessionClient>(GameInstance->GetOnlineSession()) : nullptr;
	
	if (OnlineSession)
	{
		if (!OnOnlineSessionSearchCompleteDelegateHandle.IsValid())
		{
			OnOnlineSessionSearchCompleteDelegateHandle = OnlineSession->OnSessionSearchComplete.AddUObject(this, &ThisClass::OnOnlineSessionSearchComplete);
		}		
		
		bSuccess = OnlineSession->StartOnlineSessionSearch();
	}

	return bSuccess;
}

void UMainMenuWidget::OnOnlineSessionSearchComplete(bool bWasSuccessful)
{
	OnFindMatchFinished();

	UAtomGameInstance* GameInstance = Cast<UAtomGameInstance>(GetWorld()->GetGameInstance());
	UAtomOnlineSessionClient* OnlineSession = (GameInstance != nullptr) ? Cast<UAtomOnlineSessionClient>(GameInstance->GetOnlineSession()) : nullptr;	

	if (OnlineSession)
	{
		OnlineSession->OnSessionSearchComplete.Remove(OnOnlineSessionSearchCompleteDelegateHandle);
		OnOnlineSessionSearchCompleteDelegateHandle.Reset();

		// If we found a session, join it. If not, create a new one.
		TSharedPtr<FOnlineSessionSearch> SearchResult = OnlineSession->GetSearchSettings();
		if (SearchResult->SearchResults.Num() > 0)
		{
			GameInstance->JoinSession(GetOwningLocalPlayer(), 0);
		}
		else
		{
			GameInstance->CreateSession();
		}
	}	
}
