// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/GameMode.h"
#include "AtomGameMode.generated.h"

/**
 * 
 */
UCLASS(Config=Game)
class PROJECTATOMVR_API AAtomGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	AAtomGameMode();
	
	UFUNCTION(BlueprintNativeEvent, Category = AtomGameMode)
	void ScoreKill(APlayerController* Killer, APlayerController* Victim);

	virtual void RequestCharacterChange(AAtomPlayerController* Controller, TSubclassOf<class AAtomCharacter> Character);

	TSubclassOf<class UGameModeUISubsystem> GetUIClass() const;

protected:
	UFUNCTION(BlueprintNativeEvent, Category = AtomGameMode)
	bool IsCharacterChangeAllowed(class AAtomPlayerController* Controller) const;

	/** AGameMode Interface Begin */
	virtual bool ReadyToEndMatch_Implementation() override;
	/** AGameMode Interface End */	

	/** AGameModeBase Interface Begin */
public:
	UClass* GetDefaultPawnClassForController_Implementation(AController* InController);
	/** AGameModeBase Interface End */

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Config, Category = AtomGameMode)
	TSubclassOf<UGameModeUISubsystem> UIClass;

	/** Game time limit in seconds. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Config, Category = AtomGameMode)
	float TimeLimit;

	/** The score limit that ends the game. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Config, Category = AtomGameMode)
	int ScoreLimit;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Config, Category = AtomGameMode)
	int32 MinPlayers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Config, Category = AtomGameMode)
	int32 MaxPlayers;
};
