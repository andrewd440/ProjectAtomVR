// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "AtomBaseGameMode.h"
#include "AtomGameMode.generated.h"

/**
 * 
 */
UCLASS(Config=Game)
class PROJECTATOMVR_API AAtomGameMode : public AAtomBaseGameMode
{
	GENERATED_BODY()
	
public:
	AAtomGameMode();
	
	/**
	* Scores a kill within the game.
	* 
	* @param Killer	The killer controller. If null, will only be treated as a death to the victim. If suicided, 
	*				should be the same as victim.
	* @param Victim The victim controller.
	*/
	UFUNCTION(BlueprintNativeEvent, Category = AtomGameMode)
	void ScoreKill(AController* Killer, AController* Victim);

protected:
	/** AAtomBaseGameMode Interface Begin */
	virtual bool IsCharacterChangeAllowed_Implementation(class AAtomPlayerController* Controller) const override;
	/** AAtomBaseGameMode Interface End */

	/** AGameMode Interface Begin */
	virtual bool ReadyToEndMatch_Implementation() override;
	virtual void HandleMatchHasEnded() override;
	/** AGameMode Interface End */

	/** AGameModeBase Interface Begin */
	virtual bool ShouldSpawnAtStartSpot(AController* Player) override;
	/** AGameModeBase Interface End */

protected:
	/** Game time limit in seconds. 0 = No time limit */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Config, Category = AtomGameMode)
	float TimeLimit;

	/** The score limit that ends the game. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Config, Category = AtomGameMode)
	int ScoreLimit;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Config, Category = AtomGameMode)
	int32 MinPlayers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Config, Category = AtomGameMode)
	int32 MaxPlayers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Config, Category = AtomGameMode)
	int32 KillScore = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Config, Category = AtomGameMode)
	int32 DeathScore = 0;
};
