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

	UFUNCTION(BlueprintCallable, Category = AtomBaseGameMode)
	virtual void RequestCharacterChange(AAtomPlayerController* Controller, TSubclassOf<class AAtomCharacter> Character);

	TSubclassOf<class AVRHUD> GetVRHUDClass() const;
	
	/** AGameModeBase Interface Begin */
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	virtual void InitializeHUDForPlayer_Implementation(APlayerController* NewPlayer) override;
	/** AGameModeBase Interface End */

protected:
	UFUNCTION(BlueprintNativeEvent, Category = AtomGameMode)
	bool IsCharacterChangeAllowed(class AAtomPlayerController* Controller) const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Config, Category = Classes, meta = (DisplayName="VR HUD Class"))
	TSubclassOf<class AVRHUD> VRHUDClass;
};
