// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "UObject/NoExportTypes.h"
#include "AtomPlayerHUDProxy.generated.h"

class AVRHUD;
class AAtomPlayerState;

/**
 * Proxy object that handles the HUD representation of a player within the game.
 */
UCLASS()
class PROJECTATOMVR_API UAtomPlayerHUDProxy : public UObject
{
	GENERATED_BODY()
	
public:
	UAtomPlayerHUDProxy();

	void Initialize(AVRHUD* HUD, AAtomPlayerState* Player, TSubclassOf<class UAtomPlayerNameWidget> WidgetClass);

	void TickHUD(float DeltaTime);

	void SetNameVisible(bool bVisible);

	void SetPlayerTalkingState(bool bIsTalking);

	void NotifyPlayerChangedTeams();

	void ReceiveLocalMessage(TSubclassOf<class UAtomLocalMessage> MessageClass, const int32 MessageIndex, const FText& MessageText,
		class AAtomPlayerState* RelatedPlayerState_1, class AAtomPlayerState* RelatedPlayerState_2, UObject* OptionalObject);

	class AAtomPlayerState* GetPlayer() const { return PlayerState.Get(); }

	/** UObject Interface Begin */
	virtual class UWorld* GetWorld() const override;
	virtual void BeginDestroy() override;
	/** UObject Interface End */

protected:
	APlayerController* GetLocalPlayerController() const;

private:
	UPROPERTY(Transient)
	class UAtomPlayerNameWidget* NameWidget = nullptr;

	UPROPERTY(Transient)
	class UWidgetComponent* NameWidgetComponent = nullptr;

	TWeakObjectPtr<class AAtomPlayerState> PlayerState = nullptr;

	AVRHUD* OwningHUD = nullptr;
};
