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

	void SetLineRadius(const float Radius);

	void SetFirstLineLength(const float Length);

	/** Call this every frame to orientate the text toward the specified transform */
	void Update(const FVector OrientateToward);

protected:

	virtual void UpdateInternal(const FVector& OrientateToward, const FQuat& TowardRotation);

	void SetSecondLineLength(const float Length);

	class UStaticMeshComponent* GetFirstLineComponent() const;

	class UStaticMeshComponent* GetJointSphereComponent() const;

	class UStaticMeshComponent* GetSecondLineComponent() const;

protected:

	/** Radius of lines used for the the dock. */
	float LineRadius{ 0.1f };

	/** Length of the first line that extends to the dock. */
	float FirstLineLength{ 4.0f };

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