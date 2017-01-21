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

	int32 GetSavedTeamId() const;

	/** APlayerState Interface Begin */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void ClientInitialize(class AController* C) override;
	/** APlayerState Interface End */

protected:
	UFUNCTION()
	virtual void NotifyTeamChanged();
	AAtomCharacter* GetAtomCharacter() const;

public:
	UPROPERTY(Transient, Replicated, BlueprintReadWrite, Category = AtomPlayerState)
	int32 Kills = 0;

	UPROPERTY(Transient, Replicated, BlueprintReadWrite, Category = AtomPlayerState)
	int32 Deaths = 0;

protected:
	UPROPERTY(Transient, ReplicatedUsing=NotifyTeamChanged, BlueprintReadWrite, Category = AtomPlayerState)
	AAtomTeamInfo* Team = nullptr;

private:
	UPROPERTY(Transient)
	mutable AAtomCharacter* AtomCharacter; // Cached reference to the character. Do not use directly, use GetAtomCharacter instead.

	UPROPERTY()
	int32 SavedTeamId = -1; // Saved team id used to rejoin teams after seamless travel from lobby
};