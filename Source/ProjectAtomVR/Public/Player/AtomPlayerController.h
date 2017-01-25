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

	/** Set the client's class of VRHUD and spawns a new instance of it. If there was already a VRHUD active, it is destroyed. */
	UFUNCTION(BlueprintCallable, Category = "HUD", Reliable, Client)
	void ClientSetVRHUD(TSubclassOf<class AVRHUD> NewHUDClass);

protected:
	void OnMenuButtonPressed();

	void OnMenuClickPressed();
	void OnMenuClickReleased();

private:
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetPlayerSettings(FAtomPlayerSettings InPlayerSettings);

	UFUNCTION(Exec)
	void execChangeTeams();

	/** APlayerController Interface Begin */
public:
	virtual void SetPawn(APawn* aPawn) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetPlayer(UPlayer* InPlayer) override;
	virtual void Destroyed() override;
	virtual void SpawnDefaultHUD() override;
	virtual void GetSeamlessTravelActorList(bool bToEntry, TArray<class AActor *>& ActorList) override;
	virtual void ClientTravelInternal_Implementation(const FString& URL, enum ETravelType TravelType, bool bSeamless = false, FGuid MapPackageGuid = FGuid()) override;
protected:
	virtual void SetupInputComponent() override;
	/** APlayerController Interface End */

private:
	UPROPERTY()
	class AVRHUD* VRHUD = nullptr;

	AAtomCharacter* AtomCharacter = nullptr;

	FAtomPlayerSettings PlayerSettings;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = AtomPlayerController, meta = (AllowPrivateAccess = "true"))
	class UWidgetInteractionComponent* WidgetInteraction = nullptr;

	UPROPERTY(Replicated, BlueprintReadOnly, Transient, Category = AtomPlayerController, meta = ( AllowPrivateAccess = "True" ))
	TSubclassOf<AAtomCharacter> RequestedCharacter = nullptr;
};
