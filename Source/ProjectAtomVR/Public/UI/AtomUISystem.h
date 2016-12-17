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

	void SpawnCharacterUI();
	void DestroyCharacterUI();
	
	void CreateGameModeUI(TSubclassOf<class AGameModeBase> GameModeClass);

	/** AActor Interface Begin */
	virtual void SetOwner(AActor* NewOwner) override;
	/** AActor Interface End */


private:
	void OnLoadoutSlotChanged(ELoadoutSlotChangeType Change, int32 LoadoutIndex);

private:
	AAtomPlayerController* PlayerController;

	UPROPERTY()
	class UGameModeUISubsystem* GameModeUI;

	struct FHeroUI
	{
		TArray<AEquippableUIActor*> Equippables;			

		// Hero (UI on the hero only, i.e. health)
	} HeroUI;
};
