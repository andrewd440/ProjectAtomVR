// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "AtomUISystem.generated.h"

enum class ELoadoutSlotChangeType : uint8;
class AAtomPlayerController;

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API AAtomUISystem : public AActor
{
	GENERATED_BODY()
	
public:
	AAtomUISystem();

	AAtomPlayerController* GetPlayerController() const;
	AAtomCharacter* GetCharacter() const;

	/**
	* Creates all UIs for the controlled character.
	*/
	void CreateCharacterUI();

	/**
	* Destroys all character UIs that have been created.
	*/
	void DestroyCharacterUI();
	
	/**
	* Creates all UIs for a specified GameMode.
	*/
	void CreateGameModeUI(TSubclassOf<class AGameModeBase> GameModeClass);

	/**
	* Destroys all existing GameMode UIs that have been created.
	*/
	void DestroyGameModeUI();

	/**
	* Creates all UIs for the loaded level.
	*/
	void CreateLevelUI();

	/** AActor Interface Begin */
	virtual void SetOwner(AActor* NewOwner) override;
	virtual void Destroyed() override;
	/** AActor Interface End */


private:
	void OnLoadoutSlotChanged(ELoadoutSlotChangeType Change, int32 LoadoutIndex);

private:
	AAtomPlayerController* PlayerController;

	UPROPERTY()
	class UGameModeUISubsystem* GameModeUI;

	UPROPERTY()
	class ALevelUIManager* LevelUI = nullptr;

	struct FHeroUI
	{
		TArray<AEquippableUIActor*> Equippables;			

		// Hero (UI on the hero only, i.e. health)
	} HeroUI;
};
