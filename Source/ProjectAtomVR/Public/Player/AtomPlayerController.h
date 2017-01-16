// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/PlayerController.h"
#include "AtomPlayerSettings.h"
#include "AtomPlayerController.generated.h"

class AAtomCharacter;

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API AAtomPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AAtomPlayerController();

	AAtomCharacter* GetCharacter() const;

	const FAtomPlayerSettings& GetPlayerSettings() const;

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRequestCharacterChange(TSubclassOf<AAtomCharacter> CharacterClass);

	/**
	* Sets the requested character for this controller. Should only be set by the active game mode.
	*/
	void SetRequestedCharacter(TSubclassOf<AAtomCharacter> CharacterClass);

	/**
	* Gets the requested character for this controller.
	*/
	TSubclassOf<AAtomCharacter> GetRequestedCharacter() const;	

	/** Handle respawn */
	virtual void UnFreeze() override;

	void CreateCharacterUI();

protected:
	void CreateUISystem();

	void OnMenuButtonPressed();

	void OnMenuClickPressed();
	void OnMenuClickReleased();

private:
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetPlayerSettings(FAtomPlayerSettings InPlayerSettings);

	/** APlayerController Interface Begin */
public:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void SetPawn(APawn* aPawn) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetPlayer(UPlayer* InPlayer) override;
	virtual void ReceivedGameModeClass(TSubclassOf<class AGameModeBase> GameModeClass) override;
	virtual void NotifyLoadedWorld(FName WorldPackageName, bool bFinalDest) override;
	virtual void Destroyed() override;
protected:
	virtual void SetupInputComponent() override;
	/** APlayerController Interface End */

private:
	UPROPERTY()
	class AAtomUISystem* UISystem = nullptr;

	AAtomCharacter* AtomCharacter = nullptr;

	FAtomPlayerSettings PlayerSettings;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = AtomPlayerController, meta = (AllowPrivateAccess = "true"))
	class UWidgetInteractionComponent* WidgetInteraction = nullptr;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = AtomPlayerController, meta = ( AllowPrivateAccess = "True" ))
	TSubclassOf<AAtomCharacter> RequestedCharacter = nullptr;
};
