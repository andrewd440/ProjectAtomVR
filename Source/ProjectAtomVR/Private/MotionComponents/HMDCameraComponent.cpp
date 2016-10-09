// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "HMDCameraComponent.h"




UHMDCameraComponent::UHMDCameraComponent(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{

}

FVector UHMDCameraComponent::GetRelativeHeadLocation() const
{
	constexpr float DistanceToHeadCenter = 17.f;  // ~ distance from the HMD to the center of the players head

	const FVector VectorToHead = -RelativeRotation.Vector() * DistanceToHeadCenter;
	return RelativeLocation + VectorToHead;
}

FVector UHMDCameraComponent::GetWorldHeadLocation() const
{
	return ComponentToWorld.TransformPosition(GetRelativeHeadLocation());
}