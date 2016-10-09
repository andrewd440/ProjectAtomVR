// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Components/CapsuleComponent.h"
#include "HMDCapsuleComponent.generated.h"

/**
 * Supported Attachment:
 *	VRRoot
 *		|- UCameraComponent
 *			|- UHMDCapsuleComponent
 */
UCLASS()
class PROJECTATOMVR_API UHMDCapsuleComponent : public UCapsuleComponent
{
	GENERATED_BODY()
	
public:
	UHMDCapsuleComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** UCapsuleComponent Interface Begin */
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	virtual void UpdateBodySetup() override;
	/** UCapsuleComponent Interface End */	

	/** UPrimitiveComponent Interface Begin */
protected:
	virtual bool MoveComponentImpl(const FVector& Delta, const FQuat& NewRotation, bool bSweep, FHitResult* OutHit = NULL, EMoveComponentFlags MoveFlags = MOVECOMP_NoFlags, ETeleportType Teleport = ETeleportType::None) override;
	/** UPrimitiveComponent Interface End */

	/** UActorComponent Interface Begin */
public:
	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	virtual FMatrix GetRenderMatrix() const override;

	/** UActorComponent Interface End */

private:

	/**
	* Updates the collision offset based on the current camera location.
	* This component will be moved in the event that collisions take place as a result
	* of the new collision offset.
	*/
	void UpdateCollisionOffset();

private:
	class UHMDCameraComponent* Camera = nullptr;

	FVector CollisionOffset = FVector::ZeroVector;

public:
	/** Gets the collision offset relative to this component */
	FVector GetRelativeCollisionOffset() const;

	/** Gets the collision offset (vector) in world space */
	FVector GetWorldCollisionOffset() const;
};

FORCEINLINE FVector UHMDCapsuleComponent::GetRelativeCollisionOffset() const { return CollisionOffset; }
FORCEINLINE FVector UHMDCapsuleComponent::GetWorldCollisionOffset() const { return ComponentToWorld.TransformVector(CollisionOffset); }