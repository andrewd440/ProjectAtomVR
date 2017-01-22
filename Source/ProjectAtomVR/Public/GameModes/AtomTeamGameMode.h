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

	bool ChangeTeams(AController* Controller, int32 TeamId);

protected:
	AAtomTeamInfo* ChooseBestTeam(const AController* Controller) const;		

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
protected:
	virtual void UpdateGameplayMuteList(APlayerController* aPlayer) override;
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal = TEXT("")) override;
	/** AGameModeBase Interface End */

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AtomGameMode)
	TArray<FLinearColor> TeamColors; /** Colors assigned to each team */

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AtomGameMode)
	int32 TeamCount = 2; /** Number of teams for the game */
};
