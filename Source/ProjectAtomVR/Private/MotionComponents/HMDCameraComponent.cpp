// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "HMDCameraComponent.h"


static constexpr float DistanceToHeadCenter = 17.f;  // ~ distance from the HMD to the center of the players head
static constexpr float DistanceToNeckBase = 15.f; // ~ distance from the head center to the base of the neck

UHMDCameraComponent::UHMDCameraComponent(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{

}

FVector UHMDCameraComponent::GetRelativeHeadLocation() const
{
	return RelativeLocation - RelativeRotation.Vector() * DistanceToHeadCenter;
}

FVector UHMDCameraComponent::GetWorldHeadLocation() const
{
	return ComponentToWorld.TransformPosition(-FVector::ForwardVector * DistanceToHeadCenter);
}

FVector UHMDCameraComponent::GetRelativeNeckBaseLocation() const
{
	return GetRelativeHeadLocation() - RelativeRotation.RotateVector(FVector::UpVector) * DistanceToNeckBase;
}

FVector UHMDCameraComponent::GetWorldNeckBaseLocation() const
{
	return GetWorldHeadLocation() - GetUpVector() * DistanceToNeckBase;
}
