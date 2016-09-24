// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Pawn.h"
#include "HeroBase.generated.h"

UCLASS(Blueprintable, Config=Game)
class PROJECTATOMVR_API AHeroBase : public APawn
{
	GENERATED_BODY()

public:
	enum class EHandType
	{
		Left,
		Right
	};

public:
	AHeroBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void BeginPlay() override;
	
	virtual void Tick( float DeltaSeconds ) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	bool IsRightHanded() const { return bIsRightHanded; }

	virtual void MovementTeleport(const FVector& DestLocation, const FRotator& DestRotation);

	/** APawn Interface Begin */
	virtual void PostInitializeComponents() override;
	virtual UPawnMovementComponent* GetMovementComponent() const override;
	/** APawn Interface End */

protected:
	virtual void FinishTeleport(FVector DestLocation, FRotator DestRotation);

	UFUNCTION(Server, Reliable, WithValidation)
	virtual void ServerMovementTeleport(const FVector& DestLocation, const FRotator& DestRotation);

protected:

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
	class UNetMotionControllerComponent* LeftHandController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hero, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* LeftHandMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hero, meta = (AllowPrivateAccess = "true"))
	class UNetMotionControllerComponent* RightHandController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hero, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* RightHandMesh;

	/** If the player is right hand dominant. */
	UPROPERTY(config)
	uint32 bIsRightHanded : 1;

public:
	USceneComponent* GetVROrigin() const;
	UCameraComponent* GetCamera() const;
	USkeletalMeshComponent* GetHandMesh(EHandType Hand) const;
	UNetMotionControllerComponent* GetHandController(EHandType Hand) const;
};

FORCEINLINE USceneComponent* AHeroBase::GetVROrigin() const { return VROrigin; }
FORCEINLINE UCameraComponent* AHeroBase::GetCamera() const { return Camera; }
FORCEINLINE USkeletalMeshComponent* AHeroBase::GetHandMesh(EHandType Hand) const { return (Hand == EHandType::Left) ? LeftHandMesh : RightHandMesh; }
FORCEINLINE UNetMotionControllerComponent* AHeroBase::GetHandController(EHandType Hand) const { return (Hand == EHandType::Left) ? LeftHandController : RightHandController; }