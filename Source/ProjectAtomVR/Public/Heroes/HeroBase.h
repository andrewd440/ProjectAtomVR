// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "HeroBase.generated.h"

class AHeroHand;

UCLASS(Blueprintable, Config=Game)
class PROJECTATOMVR_API AHeroBase : public APawn
{
	GENERATED_BODY()

public:
	AHeroBase();
	
	virtual void BeginPlay() override;
	
	virtual void Tick( float DeltaSeconds ) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	/** APawn Interface Begin */
	virtual void PostInitializeComponents() override;
	/** APawn Interface End */

protected:
	UPROPERTY(EditDefaultsOnly, Category = Hero)
	TSubclassOf<AHeroHand> DominateHandTemplate;

	UPROPERTY(EditDefaultsOnly, Category = Hero)
	TSubclassOf<AHeroHand> NonDominateHandTemplate;

	//UHeroWeapon* Primary;
	//UHeroWeapon* Secondary;
	// 
	//UPROPERTY(EditInstanceOnly, Category = HeroController)
	//UHeroAbility* Ability;

private:
	/** 
	 * Represents the center of the VR play space. The camera and controllers will are
	 * moved within the world relative to this component. 
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hero, meta = (AllowPrivateAccess = "true"))
	USceneComponent* VROrigin;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hero, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;

	UPROPERTY(BlueprintReadOnly, Category = Hero, meta = (AllowPrivateAccess = "true"))
	AHeroHand* DominateHand;

	UPROPERTY(BlueprintReadOnly, Category = Hero, meta = (AllowPrivateAccess = "true"))
	AHeroHand* NonDominateHand;

	/** If the player is right hand dominant. */
	UPROPERTY(config)
	uint32 bIsRightHanded : 1;
};
