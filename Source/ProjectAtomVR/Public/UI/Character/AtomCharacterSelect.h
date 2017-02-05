// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "AtomCharacterSelect.generated.h"

class AAtomCharacter;

/**
 * UI Actor that allows the player to selected a specified character.
 */
UCLASS()
class PROJECTATOMVR_API AAtomCharacterSelect : public AActor
{
	GENERATED_BODY()
	
public:	
	AAtomCharacterSelect();

	TSubclassOf<AAtomCharacter> GetCharacterClass() const;

	/** AActor Interface Begin */
	virtual void PostInitializeComponents() override;
	/** AActor Interface End */	

protected:
	UPROPERTY(EditDefaultsOnly, Category = CharacterSelect)
	TSubclassOf<AAtomCharacter> CharacterClass;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = CharacterSelect, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = CharacterSelect, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* SelectionWidgetComponent;
};
