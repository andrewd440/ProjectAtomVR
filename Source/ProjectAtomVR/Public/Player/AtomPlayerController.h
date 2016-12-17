// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/PlayerController.h"
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

	AAtomCharacter* GetHero() const;

	/**
	* Sets the requested character for this controller. Should only be set by the active game mode.
	*/
	void SetRequestedCharacter(TSubclassOf<AAtomCharacter> CharacterClass);

	/**
	* Gets the requested character for this controller.
	*/
	TSubclassOf<AAtomCharacter> GetRequestedCharacter() const;

	/**
	* Checks if the player is right handed.
	*/
	bool IsRightHanded() const;
	
protected:
	void CreateUISystem();

	UFUNCTION(Exec)
	void execRequestCharacterChange(FString Name);

	void OnMenuButtonPressed();

private:
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRequestCharacterChange(TSubclassOf<AAtomCharacter> CharacterClass);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetIsRightHanded(bool InbIsRightHanded);

	/** APlayerController Interface Begin */
public:
	virtual void PostInitializeComponents() override;
	virtual void SetPawn(APawn* aPawn) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetPlayer(UPlayer* InPlayer) override;
protected:
	virtual void SetupInputComponent() override;
	/** APlayerController Interface End */

protected:
	UPROPERTY(Replicated, BlueprintReadOnly)
	uint32 bIsRightHanded : 1;

private:
	UPROPERTY()
	class UAtomUISystem* UISystem = nullptr;
	AAtomCharacter* AtomCharacter = nullptr;

	UPROPERTY(Replicated, BlueprintReadOnly, meta = ( AllowPrivateAccess = "True" ))
	TSubclassOf<AAtomCharacter> RequestedCharacter = nullptr;
};
