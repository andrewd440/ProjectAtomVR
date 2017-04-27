// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "AtomBaseGameMode.h"
#include "AtomGameMode.generated.h"

namespace MatchState
{
	extern PROJECTATOMVR_API const FName Countdown; // In countdown to round start. Pawns are spawned but are turned off.
	extern PROJECTATOMVR_API const FName Intermission; // In intermission between rounds
	extern PROJECTATOMVR_API const FName ExitingIntermission; // Exiting intermission between rounds
}

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
	void RegisterKill(AController* Killer, AController* Victim);	

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = AtomGameMode)
	float ModifyDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* Inflictor, AController* Reciever) const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = AtomGameMode)
	bool CanDamage(AController* Inflictor, AController* Reciever) const;			

protected:
	UFUNCTION(BlueprintNativeEvent, Category = AtomGameMode)
	void ScoreKill(AAtomPlayerState* Killer, AAtomPlayerState* Victim);

	UFUNCTION(BlueprintNativeEvent, Category = AtomGameMode)
	void ScoreDeath(AAtomPlayerState* Killer, AAtomPlayerState* Victim);

	/** Checks if a playerstart is valid for a specified player. */
	virtual bool IsValidPlayerStart(AController* Player, APlayerStart* PlayerStart);

	/** Applies playlist settings to the gamemode. Called if the gamemode is loaded with the bUsePlaylist url flag. */
	virtual void ApplyPlaylistSettings(const struct FPlaylistItem& Playlist);
	
	/** Called when the match has ended to travel to the next map. Default behavior is to travel to the multiplayer lobby. */
	virtual void TravelToNextMatch();

	/** Called at the start of each round to initialize or restart actors needed for each round. */
	virtual void InitRound();

	virtual void InitGameStateForRound(AAtomGameState* GameState);

	virtual void InitPlayerStateForRound(AAtomPlayerState* PlayerState);

	/** Called after each game objective is initialized. */
	virtual void OnObjectiveInitialized(class AAtomGameObjective* Objective);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = AtomGameMode)
	void CheckForGameWinner(AAtomPlayerState* Scorer);	

	/** Called when the match transitions to Countdown. */
	virtual void HandleMatchEnteredCountdown();

	UFUNCTION(BlueprintNativeEvent, Category = AtomGameMode)
	bool ReadyToEndRound();

	/** 
	 * Checks if the match is finished and there is no more rounds to be played. Called by EndRound to determine if a new round
	 * should be started. 
	 */
	virtual bool IsMatchFinished() const;

	/** Ends the current round. Enters intermission if more rounds to left to be played. Otherwise, ends the match. */
	virtual void EndRound();

	/** Called when the match transitions to Intermission. */
	virtual void HandleMatchEnteredIntermission();

	/** Called when the match transitions to LeavingIntermission. */
	virtual void HandleMatchLeavingIntermission();	

	/** AAtomBaseGameMode Interface Begin */
protected:
	virtual void CheckGameTime() override;
	virtual bool IsCharacterChangeAllowed_Implementation(class AAtomPlayerController* Controller) const override;
	/** AAtomBaseGameMode Interface End */

	/** AGameMode Interface Begin */
public:	
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual bool PlayerCanRestart_Implementation(APlayerController* Player) override;
	virtual void StartMatch() override;
	virtual void Tick(float DeltaSeconds) override;
protected:
	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchHasEnded() override;
	virtual void OnMatchStateSet() override;
private:
	virtual bool IsMatchInProgress() const override;	
	/** AGameMode Interface End */	

	/** AGameModeBase Interface Begin */
protected:
	virtual void InitGameState() override;
	virtual bool ShouldSpawnAtStartSpot(AController* Player) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	/** AGameModeBase Interface End */

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Messages)
	TSubclassOf<class UAtomDeathLocalMessage> DeathMessageClass;

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

	UPROPERTY(EditDefaultsOnly, Category = AtomGameMode)
	int32 Rounds = 3;

	UPROPERTY(BlueprintReadOnly, Config, Category = AtomGameMode)
	int32 IntermissionTime = 10; // Seconds for Intermission match state

	UPROPERTY(BlueprintReadOnly, Config, Category = AtomGameMode)
	int32 CountdownTime = 10; // Seconds for Countdown match state
	
	uint32 bFirstRoundInitialized : 1;
};
