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

	/**
	* Tries to change teams for a player. If team can not be changed, the pending team change property will be set on the 
	* player state.
	* 
	* @returns True if teams was changed.
	*/
	bool ChangeTeams(AController* Controller, int32 TeamId);	

protected:
	/**
	* Chooses the best team to assign a new player to.
	*/
	AAtomTeamInfo* ChooseBestTeam(const AController* Controller) const;		

	/**
	* Moves a player to another team. Destroys the currently possessed pawn, removes from current team, adds 
	* to new team and restarts the player.
	*/
	void MovePlayerToTeam(AController* Controller, AAtomPlayerState* PlayerState, class AAtomTeamInfo* Team);

	/** AtomGameMode Interface Begin */
public:
	virtual void Logout(AController* Exiting) override;
	virtual void InitGameState() override;
protected:
	virtual bool IsValidPlayerStart(AController* Player, APlayerStart* PlayerStart) override;
	virtual void CheckForGameWinner_Implementation(AAtomPlayerState* Scorer) override;
	/** AtomGameMode Interface End */

	/** AGameMode Interface Begin */
public:
	virtual void InitSeamlessTravelPlayer(AController* NewController) override;
	/** AGameMode Interface End */

	/** AGameModeBase Interface Begin */
public:
	virtual void GetSeamlessTravelActorList(bool bToTransition, TArray<AActor *>& ActorList) override;
protected:
	virtual void UpdateGameplayMuteList(APlayerController* aPlayer) override;
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal = TEXT("")) override;
	/** AGameModeBase Interface End */

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = TeamGameMode)
	TArray<FLinearColor> TeamColors; /** Colors assigned to each team */

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = TeamGameMode)
	int32 TeamCount = 2; /** Number of teams for the game */

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = TeamGameMode)
	uint32 bBalanceTeams : 1; /** Number of teams for the game */
};
