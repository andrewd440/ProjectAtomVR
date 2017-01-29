// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/GameMode.h"
#include "AtomGameState.h"
#include "AtomBaseGameMode.generated.h"

/**
 * 
 */
UCLASS(Config = Game)
class PROJECTATOMVR_API AAtomBaseGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	AAtomBaseGameMode();

	UFUNCTION(BlueprintCallable, Category = AtomBaseGameMode)
	virtual void RequestCharacterChange(AAtomPlayerController* Controller, TSubclassOf<class AAtomCharacter> Character);

	TSubclassOf<class AVRHUD> GetVRHUDClass() const;
	
	AAtomGameState* GetAtomGameState() const;

	bool ShouldDelayCharacterLoadoutCreation() const;	

protected:
	UFUNCTION(BlueprintNativeEvent, Category = AtomGameMode)
	bool IsCharacterChangeAllowed(class AAtomPlayerController* Controller) const;

	/** Called every second to perform time based event that are not needed every frame. */
	virtual void DefaultTimer();

	/** Called by DefaultTimer to check time conditions for the gamemode. */
	virtual void CheckGameTime();

	/** AGameMode Interface Begin */
	virtual void PostInitializeComponents() override;
	virtual void SetMatchState(FName NewState) override;
	/** AGameMode Interface End */	

	/** AGameModeBase Interface Begin */
public:
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	virtual void InitializeHUDForPlayer_Implementation(APlayerController* NewPlayer) override;
protected:
	virtual void HandleMatchHasEnded() override;
	/** AGameModeBase Interface End */

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Config, Category = Classes, meta = (DisplayName="VR HUD Class"))
	TSubclassOf<class AVRHUD> VRHUDClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AtomGameMode)
	uint32 bDelayCharacterLoadoutCreation : 1;

private:
	FTimerHandle TimerHandle_DefaultTimer;
};

FORCEINLINE AAtomGameState* AAtomBaseGameMode::GetAtomGameState() const
{
	check(Cast<AAtomGameState>(GameState));
	return static_cast<AAtomGameState*>(GameState);
}
