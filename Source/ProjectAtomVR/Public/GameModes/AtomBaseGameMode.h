// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/GameMode.h"
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

	virtual void RequestCharacterChange(AAtomPlayerController* Controller, TSubclassOf<class AAtomCharacter> Character);

	TSubclassOf<class UGameModeUISubsystem> GetUIClass() const;
	
	/** AGameModeBase Interface Begin */
	UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	/** AGameModeBase Interface End */

protected:
	UFUNCTION(BlueprintNativeEvent, Category = AtomGameMode)
	bool IsCharacterChangeAllowed(class AAtomPlayerController* Controller) const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Config, Category = AtomGameMode)
	TSubclassOf<UGameModeUISubsystem> UIClass;
};
