// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "AtomFloatingDock.generated.h"

/**
 * Draws a 3D line dock in the world.
 * Mostly from VREditorFloatingText. 
 */
UCLASS()
class PROJECTATOMVR_API AAtomFloatingDock : public AActor
{
	GENERATED_BODY()
	
public:

	/** Default constructor that sets up CDO properties */
	AAtomFloatingDock();

	// AActor overrides
	virtual void PostActorCreated() override;

	/** Sets the opacity of the actor */
	virtual void SetOpacity(const float Opacity);

	void SetExtensionSpeed(const float Speed);

	void SetLineRadius(const float Radius);

	void SetFirstLineLength(const float Length);

	/**
	* Extends the dock and displays any content held in the dock.
	*/
	virtual void Activate();

	/**
	* True if the dock is extended and it's content is visible.
	*/
	bool IsActive() const;

	/**
	* Hides all dock content and retracts the dock.
	*/
	virtual void Deactivate(const float InDelay = 0.f);

	/** Call this every frame to orientate the text toward the specified transform */
	void Update(const float DeltaTime, const FVector OrientateToward);	

protected:

	/** 
	 * Called on update to extend the dock. 
	 * @return True to indicate the dock is fully extended.
	 */
	virtual bool StepExtension(const float DeltaTime);

	/** 
	 * Called on update to retract the dock. 
	 * @return True to indicate the dock is fully retracted.
	 */
	virtual bool StepRetraction(const float DeltaTime);

	/** Called when the dock has been fully extended after activation. */
	virtual void PostExtended();

	/** Called when the dock has been fully retracted after deactivation. */
	virtual void PostRetracted();

	virtual void UpdateInternal(const FVector& OrientateToward, const FQuat& TowardRotation);

	void SetSecondLineLength(const float Length);

	class UStaticMeshComponent* GetFirstLineComponent() const;

	class UStaticMeshComponent* GetJointSphereComponent() const;

	class UStaticMeshComponent* GetSecondLineComponent() const;

	/** AActor Interface Begin */
public:
	virtual void LifeSpanExpired() override;
	/** AActor Interface End */

protected:	

	/** Radius of lines used for the the dock. */
	float LineRadius = 0.1f;

	/** Length of the first line that extends to the dock. */
	float FirstLineLength = 4.0f;

	float ExtensionSpeed = 20.f;

	FTimerHandle DeactivationTimerHandle;

	// Flags to indicate current states
	uint32 bIsActive : 1;
	uint32 bIsExtended : 1;
	uint32 bIsRetracted : 1;

private:

	/** Scene component root of this actor */
	UPROPERTY()
	class USceneComponent* SceneComponent;

	/** First line segment component.  Starts at the designation location, goes toward the line connection point. */
	UPROPERTY()
	class UStaticMeshComponent* FirstLineComponent;

	/** Sphere that connects the two line segments and makes the joint look smooth and round */
	UPROPERTY()
	class UStaticMeshComponent* JointSphereComponent;

	/** Second line segment component.  Starts at the connection point and goes toward the 3D text. */
	UPROPERTY()
	class UStaticMeshComponent* SecondLineComponent;

	/** Material to use for the line meshes */
	UPROPERTY()
	class UMaterialInterface* LineMaterial;

	/** Dynamic material instance for fading lines in and out */
	UPROPERTY(transient)
	class UMaterialInstanceDynamic* LineMaterialMID;
};

FORCEINLINE class UStaticMeshComponent* AAtomFloatingDock::GetFirstLineComponent() const { return FirstLineComponent; }

FORCEINLINE class UStaticMeshComponent* AAtomFloatingDock::GetJointSphereComponent() const { return JointSphereComponent; }

FORCEINLINE class UStaticMeshComponent* AAtomFloatingDock::GetSecondLineComponent() const { return SecondLineComponent; }

FORCEINLINE bool AAtomFloatingDock::IsActive() const { return bIsActive; }