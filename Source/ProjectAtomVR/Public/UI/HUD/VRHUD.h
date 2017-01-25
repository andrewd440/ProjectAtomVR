// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "VRHUD.generated.h"

enum class ELoadoutSlotChangeType : uint8;
class AAtomPlayerController;
class AAtomCharacter;

DECLARE_LOG_CATEGORY_EXTERN(LogVRHUD, Log, All);

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API AVRHUD : public AActor
{
	GENERATED_BODY()
	
public:
	AVRHUD();

	AAtomPlayerController* GetPlayerController() const;

	AAtomCharacter* GetCharacter() const;

	/**
	* Called by the owning player controller when the possessed pawn is changed.
	*/
	void OnCharacterChanged(AAtomCharacter* OldCharacter);

	/** AActor Interface Begin */
	virtual void SetOwner(AActor* NewOwner) override;
	virtual void Destroyed() override;
	/** AActor Interface End */

protected:
	/**
	* Creates all UIs for the controlled character.
	*/
	void SpawnLoadoutActors();

	/**
	* Destroys all character UIs that have been created.
	*/
	void DestroyLoadoutActors(AAtomCharacter* OldCharacter);

private:
	void OnLoadoutSlotChanged(ELoadoutSlotChangeType Change, int32 LoadoutIndex);

private:
	AAtomPlayerController* PlayerController = nullptr;

	UPROPERTY()
	TArray<AEquippableHUDActor*> LoadoutActors;			
};
