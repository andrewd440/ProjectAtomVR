// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Pawn.h"
#include "HeroBase.generated.h"

UCLASS(Blueprintable, Config=Game)
class PROJECTATOMVR_API AHeroBase : public APawn
{
	GENERATED_BODY()
	
public:
	/** Name of the MovementType component. Use this name if you want to use a different class (with ObjectInitializer.SetDefaultSubobjectClass). */
	static const FName MovementTypeComponentName;

public:
	AHeroBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void BeginPlay() override;
	
	virtual void Tick( float DeltaSeconds ) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	bool IsRightHanded() const { return bIsRightHanded; }

	/** APawn Interface Begin */
	virtual void PostInitializeComponents() override;
	virtual UPawnMovementComponent* GetMovementComponent() const override;
	/** APawn Interface End */

protected:
	UPROPERTY(EditDefaultsOnly, Category = Hero)
	TSubclassOf<class AHeroHand> DominateHandTemplate;

	UPROPERTY(EditDefaultsOnly, Category = Hero)
	TSubclassOf<class AHeroHand> NonDominateHandTemplate;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hero, meta = (AllowPrivateAccess = "true"))
	class UHeroMovementComponent* MovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hero, meta = (AllowPrivateAccess = "true"))
	class UHeroMovementType* MovementType;

	UPROPERTY(BlueprintReadOnly, Category = Hero, meta = (AllowPrivateAccess = "true"))
	class AHeroHand* DominateHand;

	UPROPERTY(BlueprintReadOnly, Category = Hero, meta = (AllowPrivateAccess = "true"))
	class AHeroHand* NonDominateHand;

	/** If the player is right hand dominant. */
	UPROPERTY(config)
	uint32 bIsRightHanded : 1;

public:
	FORCEINLINE USceneComponent* GetVROrigin() const { return VROrigin; }
	FORCEINLINE UCameraComponent* GetCamera() const { return Camera; }
	FORCEINLINE AHeroHand* GetDominateHand() const { return DominateHand; }
	FORCEINLINE AHeroHand* GetNonDominateHand() const { return NonDominateHand; }
	FORCEINLINE UHeroMovementType* GetMovementType() const { return MovementType; }
};
