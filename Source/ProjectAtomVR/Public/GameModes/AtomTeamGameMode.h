// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameModes/AtomGameMode.h"
#include "AtomTeamGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API AAtomTeamGameMode : public AAtomGameMode
{
	GENERATED_BODY()
	
public:
	AAtomTeamGameMode();		

protected:
	AAtomTeamInfo* ChooseBestTeam(const AController* Controller) const;	

	/** AtomGameMode Interface Begin */
public:
	virtual void Logout(AController* Exiting) override;
	virtual void InitGameState() override;
protected:
	virtual bool IsValidPlayerStart(AController* Player, APlayerStart* PlayerStart) override;
	virtual void CheckForGameWinner_Implementation(AAtomPlayerState* Scorer) override;
	/** AtomGameMode Interface End */

	/** AGameModeBase Interface Begin */
	virtual void UpdateGameplayMuteList(APlayerController* aPlayer) override;
	virtual void GenericPlayerInitialization(AController* C) override;
	/** AGameModeBase Interface End */

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AtomGameMode)
	TArray<FLinearColor> TeamColors; /** Colors assigned to each team */

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AtomGameMode)
	int32 TeamCount = 2; /** Number of teams for the game */
};
