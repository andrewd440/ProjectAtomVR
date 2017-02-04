// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/PlayerState.h"
#include "AtomPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API AAtomPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	AAtomPlayerState();

	AAtomTeamInfo* GetTeam() const;
	void SetTeam(AAtomTeamInfo* InTeam);

	uint32 GetSavedTeamId() const;

	AAtomCharacter* GetAtomCharacter() const;

	/** 
	 * Sends a request to the server to switch teams. 
	 * @param TeamId The id for the team. -1 to switch to the next team.
	 */
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category=AtomPlayerState)
	void ServerRequestTeamChange(int32 RequestedTeam);

	/** Sets the team id for a pending team change that has been requested. Only set on server by gamemode. */
	void SetPendingTeamChange(uint32 InTeamId);

	/** Gets the team id for a pending team changes that has been requested. */
	uint32 GetPendingTeamChange() const;

protected:
	UFUNCTION()
	virtual void NotifyTeamChanged();

	UFUNCTION()
	void OnRep_PendingTeamChange();	


	/** APlayerState Interface Begin */
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void ClientInitialize(class AController* C) override;
	virtual void Reset() override;
protected:
	virtual void CopyProperties(APlayerState* PlayerState) override;
	/** APlayerState Interface End */

public:
	UPROPERTY(Transient, Replicated, BlueprintReadWrite, Category = AtomPlayerState)
	int32 Kills = 0;

	UPROPERTY(Transient, Replicated, BlueprintReadWrite, Category = AtomPlayerState)
	int32 Deaths = 0;

protected:
	UPROPERTY(Transient, ReplicatedUsing=NotifyTeamChanged, BlueprintReadWrite, Category = AtomPlayerState)
	AAtomTeamInfo* Team = nullptr;

	UPROPERTY(ReplicatedUsing=OnRep_PendingTeamChange)
	uint8 PendingTeamChange = 255;

	UPROPERTY()
	uint8 SavedTeamId = 255; // Saved team id used to rejoin teams after seamless travel from lobby. 255 for no team.

private:
	UPROPERTY(Transient)
	mutable AAtomCharacter* AtomCharacter; // Cached reference to the character. Do not use directly, use GetAtomCharacter instead.
};