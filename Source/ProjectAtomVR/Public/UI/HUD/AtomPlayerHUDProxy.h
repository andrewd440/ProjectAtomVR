// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "UObject/NoExportTypes.h"
#include "AtomPlayerHUDProxy.generated.h"

/**
 * Proxy object that handles the HUD representation of a player within the game.
 */
UCLASS()
class PROJECTATOMVR_API UAtomPlayerHUDProxy : public UObject
{
	GENERATED_BODY()
	
public:
	UAtomPlayerHUDProxy();

	void Initialize(class AAtomPlayerState* Player, TSubclassOf<class UAtomPlayerNameWidget> WidgetClass);

	void TickHUD(float DeltaTime);

	void SetNameVisible(bool bVisible);

	void SetPlayerTalkingState(bool bIsTalking);

	void PlayerChangedTeams();

	class AAtomPlayerState* GetPlayer() { return PlayerState.Get(); }

	/** UObject Interface Begin */
	virtual class UWorld* GetWorld() const override;
	virtual void BeginDestroy() override;
	/** UObject Interface End */

private:
	UPROPERTY(Transient)
	class UAtomPlayerNameWidget* NameWidget = nullptr;

	UPROPERTY(Transient)
	class UWidgetComponent* NameWidgetComponent = nullptr;

	TWeakObjectPtr<class AAtomPlayerState> PlayerState = nullptr;
};
