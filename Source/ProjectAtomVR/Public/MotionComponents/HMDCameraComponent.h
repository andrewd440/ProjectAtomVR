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
};
