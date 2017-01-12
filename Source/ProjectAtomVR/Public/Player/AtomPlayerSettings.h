// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "AtomPlayerSettings.generated.h"

USTRUCT()
struct FAtomCharacterSettings
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AAtomCharacter> Character;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LoadoutOffset;
};

/**
 * 
 */
USTRUCT()
struct FAtomPlayerSettings
{
	GENERATED_USTRUCT_BODY()	
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float PlayerHeight = 175.f; // Player height in UE units (cm)	

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	uint32 bIsRightHanded : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, NotReplicated)
	TArray<FAtomCharacterSettings> CharacterSettings; // #AtomTodo Make sure this in not replicated
};
