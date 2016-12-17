// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Character.h"
#include "AtomCharacterMovementComponent.h"

#include "AtomCharacter.generated.h"

class AAtomEquippable;
class UNetMotionControllerComponent;

UCLASS(Abstract, Config=Game)
class PROJECTATOMVR_API AAtomCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AAtomCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void BeginPlay() override;
	
	virtual void Tick( float DeltaSeconds ) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	virtual void MovementTeleport(const FVector& DestLocation, const FRotator& DestRotation);

	class UHMDCapsuleComponent* GetHMDCapsuleComponent() const;

	bool IsRightHanded() const { return bIsRightHanded; }

	void SetIsRightHanded(bool InbIsRightHanded);

	virtual void Equip(AAtomEquippable* Item, const EHand Hand);

	/** Called by Equippable when the equipping process is complete. */
	virtual void OnEquipped(AAtomEquippable* Item, const EHand Hand);

	virtual void Unequip(AAtomEquippable* Item, const EHand Hand);

	/** Called by Equippable when the unequipping process is complete. */
	virtual void OnUnequipped(AAtomEquippable* Item, const EHand Hand);

	/**
	* Gets the component that controls the target location and rotation for the hand mesh.
	* This is always the detached hand mesh for the respective hands, which is only rendered
	* for local players and is used as the target for the full body meshes hands.
	*
	* When offsetting the hands from the motion controllers for any reason, this should be
	* the component that the offset is applied to.
	*/
	USceneComponent* GetHandMeshTarget(const EHand Hand) const;

	/** 
	 * Gets the default hand mesh location and rotation. 
	 * This is also the default position for the hand mesh target.
	 */
	void GetDefaultHandMeshLocationAndRotation(const EHand Hand, FVector& Location, FRotator& Rotation) const;

	/** 
	 * Gets the default hand mesh location. 
	 * This is also the default position for the hand mesh target.
	 */
	FVector GetDefaultHandMeshLocation(const EHand Hand) const;

	/** 
	 * Gets the default hand mesh rotation. 
	 * This is also the default position for the hand mesh target.
	 */
	FRotator GetDefaultHandMeshRotation(const EHand Hand) const;

	UMeshComponent* GetBodyAttachmentComponent() const;

	USkeletalMeshComponent* GetHandAttachmentComponent(const EHand Hand) const;

	FVector GetRoomScaleVelocity() const;

	/** 
	 * Plays an animation on a specified hand. The animation and mesh that it will be played on
	 * is based on if the character is locally controlled or not. Animation sequences will be
	 * played looping.
	 */
	void PlayHandAnimation(const EHand Hand, const FHandAnim& Anim);

	void StopHandAnimation(const EHand Hand, const FHandAnim& Anim);

	class UAtomLoadout* GetLoadout() const;

	/** ACharacter Interface Begin */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual FVector GetPawnViewLocation() const override;
	virtual FRotator GetViewRotation() const override;
	virtual void PostNetReceiveLocationAndRotation() override;
	/** ACharacter Interface End */

	/** APawn Interface Begin */
	virtual void PostInitializeComponents() override;
	virtual FVector GetVelocity() const override;	
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	/** APawn Interface End */

	/** AActor Interface Begin */
	virtual bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;	
	virtual void Destroyed() override;
	/** AActor Interface End */

protected:
	template <EHand Hand>
	void OnEquipPressed();
	virtual void FinishTeleport(FVector DestLocation, FRotator DestRotation);

private:
	void UpdateMeshLocation(float DeltaTime);

protected:
	/** Socket on the body mesh that has the offset for the intended head mesh location. */
	UPROPERTY(EditDefaultsOnly, Category = Hero)
	FName NeckBaseSocket = NAME_None;	

	/** Room scale movement velocity of the player. */
	UPROPERTY(Transient, BlueprintReadOnly, Category = Hero)
	FVector RoomScaleVelocity = FVector::ZeroVector;

private:
	FVector NeckBaseSocketOffset = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hero, meta = (AllowPrivateAccess = "true"))
	class UHMDCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hero, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* BodyMesh;

	UPROPERTY(VisibleAnywhere, Instanced, BlueprintReadOnly, Category = Hero, meta = (AllowPrivateAccess = "true"))
	class UAtomLoadout* Loadout;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hero, meta = (AllowPrivateAccess = "true"))
	AAtomEquippable* LeftHandEquippable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hero, meta = (AllowPrivateAccess = "true"))
	UNetMotionControllerComponent* LeftHandController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hero, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* LeftHandMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hero, meta = (AllowPrivateAccess = "true"))
	USphereComponent* LeftHandTrigger;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hero, meta = (AllowPrivateAccess = "true"))
	AAtomEquippable* RightHandEquippable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hero, meta = (AllowPrivateAccess = "true"))
	UNetMotionControllerComponent* RightHandController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hero, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* RightHandMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hero, meta = (AllowPrivateAccess = "true"))
	USphereComponent* RightHandTrigger;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hero, meta = (AllowPrivateAccess = "true"))
	class UWidgetInteractionComponent* WidgetInteraction;

	struct FDefaultHandTransform
	{
		FVector Location;
		FRotator Rotation;
	};

	FDefaultHandTransform DefaultLeftHandTransform;
	FDefaultHandTransform DefaultRightHandTransform;

	/** If the player is right hand dominant. */
	uint32 bIsRightHanded : 1;

public:
	class UAtomCharacterMovementComponent* GetHeroMovementComponent() const;

	class UHMDCameraComponent* GetCamera() const;
	
	/** Gets the body mesh for the hero. This is also the mesh that loadout items are attached to. */
	UStaticMeshComponent* GetBodyMesh() const;

	AAtomEquippable* GetEquippable(EHand Hand) const;

	template <EHand Hand>
	AAtomEquippable* GetEquippable() const;

	USkeletalMeshComponent* GetHandMesh(EHand Hand) const;

	template <EHand Hand>
	USkeletalMeshComponent* GetHandMesh() const;	

	template <EHand Hand>
	UNetMotionControllerComponent* GetHandController() const;

	UNetMotionControllerComponent* GetHandController(EHand Hand) const;

	template <EHandType Hand>
	USkeletalMeshComponent* GetHandMesh() const;	

	template <EHandType Hand>
	UNetMotionControllerComponent* GetHandController() const;	

	USphereComponent* GetHandTrigger(EHand Hand) const;

	template <EHand Hand>
	USphereComponent* GetHandTrigger() const;

	class UWidgetInteractionComponent* GetWidgetInteraction() const;
};

template <EHand Hand>
FORCEINLINE USkeletalMeshComponent* AAtomCharacter::GetHandMesh() const
{
	return (Hand == EHand::Left) ? LeftHandMesh : RightHandMesh;
}

template <EHandType Hand>
FORCEINLINE USkeletalMeshComponent* AAtomCharacter::GetHandMesh() const
{
	return (Hand == EHandType::Dominate && bIsRightHanded) ? RightHandMesh : LeftHandMesh;
}

template <EHand Hand>
FORCEINLINE UNetMotionControllerComponent* AAtomCharacter::GetHandController() const
{
	return (Hand == EHand::Left) ? LeftHandController : RightHandController;
}

template <EHandType Hand>
FORCEINLINE UNetMotionControllerComponent* AAtomCharacter::GetHandController() const
{
	return (Hand == EHandType::Dominate && bIsRightHanded) ? RightHandController : LeftHandController;
}

FORCEINLINE UHMDCameraComponent* AAtomCharacter::GetCamera() const
{
	return Camera;
}

FORCEINLINE UStaticMeshComponent* AAtomCharacter::GetBodyMesh() const
{
	return BodyMesh;
}

FORCEINLINE USkeletalMeshComponent* AAtomCharacter::GetHandMesh(EHand Hand) const
{
	return Hand == EHand::Right ? RightHandMesh : LeftHandMesh;
}

FORCEINLINE UNetMotionControllerComponent* AAtomCharacter::GetHandController(EHand Hand) const
{
	return Hand == EHand::Right ? RightHandController : LeftHandController;
}

FORCEINLINE AAtomEquippable* AAtomCharacter::GetEquippable(EHand Hand) const
{
	return (Hand == EHand::Left) ? LeftHandEquippable : RightHandEquippable;
}

template <EHand Hand>
AAtomEquippable* AAtomCharacter::GetEquippable() const
{
	return (Hand == EHand::Left) ? LeftHandEquippable : RightHandEquippable;
}

FORCEINLINE UAtomCharacterMovementComponent* AAtomCharacter::GetHeroMovementComponent() const
{
	return static_cast<UAtomCharacterMovementComponent*>(GetMovementComponent());
}

FORCEINLINE USphereComponent* AAtomCharacter::GetHandTrigger(EHand Hand) const
{
	return (Hand == EHand::Left) ? LeftHandTrigger : RightHandTrigger;
}

template <EHand Hand>
USphereComponent* AAtomCharacter::GetHandTrigger() const
{
	return (Hand == EHand::Left) ? LeftHandTrigger : RightHandTrigger;
}