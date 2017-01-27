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

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = AtomGameMode)
	float ModifyDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* Inflictor, AController* Reciever) const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = AtomGameMode)
	bool CanDamage(AController* Inflictor, AController* Reciever) const;

protected:
	virtual bool IsValidPlayerStart(AController* Player, APlayerStart* PlayerStart);
	virtual void ApplyPlaylistSettings(const struct FPlaylistItem& Playlist);
	virtual void TravelToNextMatch();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = AtomGameMode)
	void CheckForGameWinner(AAtomPlayerState* Scorer);

	/** AAtomBaseGameMode Interface Begin */
protected:
	virtual bool IsCharacterChangeAllowed_Implementation(class AAtomPlayerController* Controller) const override;
	/** AAtomBaseGameMode Interface End */

	/** AGameMode Interface Begin */
public:
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
protected:
	virtual bool ReadyToEndMatch_Implementation() override;
	virtual void HandleMatchHasEnded() override;
	/** AGameMode Interface End */

	/** AGameModeBase Interface Begin */
protected:
	virtual void InitGameState() override;
	virtual bool ShouldSpawnAtStartSpot(AController* Player) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
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
