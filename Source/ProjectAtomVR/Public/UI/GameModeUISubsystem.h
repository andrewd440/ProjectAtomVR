// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "UObject/NoExportTypes.h"
#include "GameModeUISubsystem.generated.h"

class AAtomUISystem;
class AAtomGameMode;

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class PROJECTATOMVR_API UGameModeUISubsystem : public UObject
{
	GENERATED_BODY()
	
public:
	UGameModeUISubsystem();

	virtual void InitializeSystem(AAtomUISystem* Owner, AAtomGameMode* GameModeCDO);
	
	/** UObject Interface Begin */
	virtual class UWorld* GetWorld() const override;
	/** UObject Interface End */

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = GameModeUI)
	uint32 bShowPlayerNames : 1;

private:
	TWeakObjectPtr<AAtomGameState> GameState;
	
	AAtomUISystem* UISystem;
};
