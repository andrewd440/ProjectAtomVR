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

protected:

	/** AGameMode Interface Begin */
	virtual bool ReadyToEndMatch_Implementation() override;
	/** AGameMode Interface End */	

protected:
	/** Game time limit in seconds. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Config, Category = AtomGameMode)
	float TimeLimit;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Config, Category = AtomGameMode)
	int32 MinPlayers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Config, Category = AtomGameMode)
	int32 MaxPlayers;
};
