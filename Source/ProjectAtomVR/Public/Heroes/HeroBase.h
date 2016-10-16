// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Character.h"
#include "HeroMovementComponent.h"

#include "HeroBase.generated.h"

UCLASS(Abstract, Config=Game)
class PROJECTATOMVR_API AHeroBase : public ACharacter
{
	GENERATED_BODY()

public:
	AHeroBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void BeginPlay() override;
	
	virtual void Tick( float DeltaSeconds ) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	virtual void MovementTeleport(const FVector& DestLocation, const FRotator& DestRotation);

	class UHMDCapsuleComponent* GetHMDCapsuleComponent() const;

	bool IsRightHanded() const { return bIsRightHanded; }

	virtual void Equip(class AHeroEquippable* Item, const EHand Hand);

	virtual void Unequip(class AHeroEquippable* Item, const EHand Hand);

	/** ACharacter Interface Begin */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual FVector GetPawnViewLocation() const override;
	virtual FRotator GetViewRotation() const override;
	virtual void PostNetReceiveLocationAndRotation() override;
	/** ACharacter Interface End */

	/** APawn Interface Begin */
	virtual void PostInitializeComponents() override;
	virtual FVector GetVelocity() const override;		
	/** APawn Interface End */

	/** AActor Interface Begin */
	virtual bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;
	/** AActor Interface End */

protected:
	template <EHand Hand>
	void OnEquipPressed();
	virtual void FinishTeleport(FVector DestLocation, FRotator DestRotation);

	UFUNCTION(Server, WithValidation, Reliable)
	void ServerEquip(class AHeroEquippable* Item, const EHand Hand);

	UFUNCTION()
	void OnRep_LeftHandEquippable();

	UFUNCTION()
	void OnRep_RightHandEquippable();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hero, meta = (AllowPrivateAccess = "true"))
	class UHMDCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hero, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* HeadMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hero, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* BodyMesh;

	UPROPERTY(VisibleAnywhere, Instanced, BlueprintReadOnly, Category = Hero, meta = (AllowPrivateAccess = "true"))
	class UHeroLoadout* Loadout;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_LeftHandEquippable, Category = Hero, meta = (AllowPrivateAccess = "true"))
	class AHeroEquippable* LeftHandEquippable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hero, meta = (AllowPrivateAccess = "true"))
	class UNetMotionControllerComponent* LeftHandController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hero, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* LeftHandMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_RightHandEquippable, Category = Hero, meta = (AllowPrivateAccess = "true"))
	class AHeroEquippable* RightHandEquippable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hero, meta = (AllowPrivateAccess = "true"))
	class UNetMotionControllerComponent* RightHandController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hero, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* RightHandMesh;

	/** If the player is right hand dominant. */
	UPROPERTY(config)
	uint32 bIsRightHanded : 1;

public:
	class UHeroMovementComponent* GetHeroMovementComponent() const;

	class UHMDCameraComponent* GetCamera() const;
	
	class USkeletalMeshComponent* GetBodyMesh() const;

	USkeletalMeshComponent* GetHandMesh(EHand Hand);

	template <EHand Hand>
	USkeletalMeshComponent* GetHandMesh() const;	

	template <EHand Hand>
	UNetMotionControllerComponent* GetHandController() const;

	template <EHandType Hand>
	USkeletalMeshComponent* GetHandMesh() const;	

	template <EHandType Hand>
	UNetMotionControllerComponent* GetHandController() const;	
};

template <EHand Hand>
FORCEINLINE USkeletalMeshComponent* AHeroBase::GetHandMesh() const
{
	return (Hand == EHand::Left) ? LeftHandMesh : RightHandMesh;
}

template <EHandType Hand>
FORCEINLINE USkeletalMeshComponent* AHeroBase::GetHandMesh() const
{
	return (Hand == EHandType::Dominate && bIsRightHanded) ? RightHandMesh : LeftHandMesh;
}

template <EHand Hand>
FORCEINLINE UNetMotionControllerComponent* AHeroBase::GetHandController() const
{
	return (Hand == EHand::Left) ? LeftHandController : RightHandController;
}

template <EHandType Hand>
FORCEINLINE UNetMotionControllerComponent* AHeroBase::GetHandController() const
{
	return (Hand == EHandType::Dominate && bIsRightHanded) ? RightHandController : LeftHandController;
}

FORCEINLINE UHMDCameraComponent* AHeroBase::GetCamera() const
{
	return Camera;
}

FORCEINLINE USkeletalMeshComponent* AHeroBase::GetBodyMesh() const
{
	return BodyMesh;
}

FORCEINLINE USkeletalMeshComponent* AHeroBase::GetHandMesh(EHand Hand)
{
	return Hand == EHand::Right ? RightHandMesh : LeftHandMesh;
}

FORCEINLINE UHeroMovementComponent* AHeroBase::GetHeroMovementComponent() const
{
	return static_cast<UHeroMovementComponent*>(GetMovementComponent());
}