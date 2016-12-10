// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "MotionComponents/NetCameraComponent.h"
#include "HMDCameraComponent.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UHMDCameraComponent : public UNetCameraComponent
{
	GENERATED_BODY()

public:
	UHMDCameraComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	/**
	* Gets the relative, to attached parent, head position of the player using the HMD. Not the same as the camera location. 
	* This location is calculated by offsetting the HMD location by a vector from the front of the tracked HMD to the center
	* of the player's head.
	*/
	FVector GetRelativeHeadLocation() const;

	/**
	* Gets the world head position of the player using the HMD. Not the same as the camera location.
	* This location is calculated by offsetting the HMD location by a vector from the front of the tracked HMD to the center
	* of the player's head.
	*/
	FVector GetWorldHeadLocation() const;

	/**
	* Gets the relative, to attached parent, location of the base of the player's neck using the HMD.
	* This location is calculated by offsetting the head center location by the HMD down direction.
	*/
	FVector GetRelativeNeckBaseLocation() const;

	/**
	* Gets the world location of the base of the player's neck using the HMD.
	* This location is calculated by offsetting the head center location by the HMD down direction.
	*/
	FVector GetWorldNeckBaseLocation() const;

	/**
	* Calculates the pitch and roll of a torso attached to this camera.
	* 
	* @returns True if the forward direction is inverted. This is determined based on the up and right
	*	       directions of the camera.		
	*/
	bool CalculateTorsoPitchAndRoll(FRotator& TorsoPitchRollOut) const;
};
