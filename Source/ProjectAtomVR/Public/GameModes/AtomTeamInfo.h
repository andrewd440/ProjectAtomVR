// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Info.h"
#include "AtomTeamInfo.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API AAtomTeamInfo : public AInfo
{
	GENERATED_BODY()

public:
	static constexpr uint8 INDEX_NO_TEAM = 255;

public:
	AAtomTeamInfo();

	/** Adds a member from this team. */
	void AddToTeam(AController* Controller);

	/** Removes a member from this team. */
	void RemoveFromTeam(AController* Controller);

	const TArray<AController*>& GetTeamMembers() const;

	/** Gets the number of team members on the team */
	int32 Size() const { return TeamMembers.Num(); }

	/** AActor Interface Begin */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	/** AActor Interface End */	

public:
	UPROPERTY(BlueprintReadOnly, Replicated, Category = AtomTeamInfo)
	FLinearColor TeamColor;

	UPROPERTY(BlueprintReadOnly, Category = AtomTeamInfo)
	uint8 TeamId = INDEX_NO_TEAM;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = AtomTeamInfo)
	int32 Score = 0;

protected:
	UPROPERTY()
	TArray<AController*> TeamMembers; // Maintained on server and remotes
};
