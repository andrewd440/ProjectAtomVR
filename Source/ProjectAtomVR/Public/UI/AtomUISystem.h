// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "AtomUISystem.generated.h"

enum class ELoadoutSlotChangeType : uint8;
class AAtomPlayerController;

/**
 * Some UI systems require a 'UILocator' tagged actor to be in the level to be placed in the correct location.
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
	* Gets the UI locator actor in the current level. This is the actor tag with
	* 'UILocator' within the level that manages static UI placement.
	* @returns
	*/
	AActor* GetUILocatorActor() const;

	USceneComponent* FindFirstUILocator(const FName Tag) const;
	TArray<USceneComponent*> FindAllUILocators(const FName Tag) const;

	/** AActor Interface Begin */
	virtual void SetOwner(AActor* NewOwner) override;
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void Destroyed() override;
	/** AActor Interface End */


private:
	void OnLoadoutSlotChanged(ELoadoutSlotChangeType Change, int32 LoadoutIndex);

private:
	AAtomPlayerController* PlayerController;

	UPROPERTY()
	AActor* UILocator;

	UPROPERTY()
	class UGameModeUISubsystem* GameModeUI;

	struct FHeroUI
	{
		TArray<AEquippableUIActor*> Equippables;			

		// Hero (UI on the hero only, i.e. health)
	} HeroUI;
};
