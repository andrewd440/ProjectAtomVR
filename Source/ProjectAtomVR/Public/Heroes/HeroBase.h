// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Character.h"
#include "HeroMovementComponent.h"

#include "HeroBase.generated.h"

UCLASS(Blueprintable, Config=Game)
class PROJECTATOMVR_API AHeroBase : public ACharacter
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

	/** ACharacter Interface Begin */
	virtual FVector GetPawnViewLocation() const override;
	virtual FRotator GetViewRotation() const override;
	virtual void PostNetReceiveLocationAndRotation() override;
	/** ACharacter Interface End */

	/** APawn Interface Begin */
	virtual void PostInitializeComponents() override;
	virtual FVector GetVelocity() const override;
	/** APawn Interface End */

protected:
	virtual void FinishTeleport(FVector DestLocation, FRotator DestRotation);

protected:

	//UHeroWeapon* Primary;
	//UHeroWeapon* Secondary;
	// 
	//UPROPERTY(EditInstanceOnly, Category = HeroController)
	//UHeroAbility* Ability;
	// 

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hero, meta = (AllowPrivateAccess = "true"))
	class UHMDCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hero, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* HeadMesh;

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
	class UHMDCameraComponent* GetCamera() const;
	class USkeletalMeshComponent* GetHandMesh(EHandType Hand) const;
	class UNetMotionControllerComponent* GetHandController(EHandType Hand) const;
	class UHeroMovementComponent* GetHeroMovementComponent() const;
};

FORCEINLINE UHMDCameraComponent* AHeroBase::GetCamera() const { return Camera; }
FORCEINLINE USkeletalMeshComponent* AHeroBase::GetHandMesh(EHandType Hand) const { return (Hand == EHandType::Left) ? LeftHandMesh : RightHandMesh; }
FORCEINLINE UNetMotionControllerComponent* AHeroBase::GetHandController(EHandType Hand) const { return (Hand == EHandType::Left) ? LeftHandController : RightHandController; }
FORCEINLINE UHeroMovementComponent* AHeroBase::GetHeroMovementComponent() const { return static_cast<UHeroMovementComponent*>(GetMovementComponent()); }