// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Engine/LocalPlayer.h"
#include "AtomLocalPlayer.generated.h"

/**
 * 
 */
UCLASS(Config=Game)
class PROJECTATOMVR_API UAtomLocalPlayer : public ULocalPlayer
{
	GENERATED_BODY()
	
public:
	void SetPlayerHeight(const float Height);
	float GetPlayerHeight() const;

	void SetIsRightHanded(bool InbIsRightHanded);
	bool GetIsRightHanded() const;

protected:

	UPROPERTY(BlueprintReadWrite, Config, Category = AtomPlayer)
	float PlayerHeight = 175.f; // Player height in UE units (cm)	

	UPROPERTY(BlueprintReadWrite, Config, Category = AtomPlayer)
	uint32 bIsRightHanded : 1;
};
