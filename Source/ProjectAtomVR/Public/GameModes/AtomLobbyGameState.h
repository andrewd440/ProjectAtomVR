// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameModes/AtomGameState.h"
#include "AtomPlaylistManager.h"
#include "AtomLobbyGameState.generated.h"

/**
 * GameState used by the lobby game mode.
 */
UCLASS()
class PROJECTATOMVR_API AAtomLobbyGameState : public AAtomGameState
{
	GENERATED_BODY()

public:
	AAtomLobbyGameState();

	void SetNextPlaylistItem(const FPlaylistItem& Item);

	const FPlaylistItem& GetNextPlaylistItem() const;

	float GetPreGameStartTimeStamp() const { return PreGameStartTimeStamp; }

protected:
	UFUNCTION()
	void OnRep_NextPlaylistItem();	

	/** AGameStateBase Interface Begin */
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:
	virtual void HandleMatchHasEnded() override;
	/** AGameStateBase Interface End */	

protected:
	/** The next playlist item */
	UPROPERTY(ReplicatedUsing=OnRep_NextPlaylistItem, Transient)
	FPlaylistItem NextPlaylistItem;

	UPROPERTY(Transient, BlueprintReadOnly)
	float PreGameStartTimeStamp = 0; // Time stamp for when the pregame timer started
};
