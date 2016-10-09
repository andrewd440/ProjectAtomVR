// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "HMDCameraComponent.h"




UHMDCameraComponent::UHMDCameraComponent(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{

}

FVector UHMDCameraComponent::GetRelativeHeadLocation() const
{
	return RelativeLocation + -RelativeRotation.Vector() * DistanceToHeadCenter;
}

FVector UHMDCameraComponent::GetWorldHeadLocation() const
{
	return ComponentToWorld.TransformPosition(-FVector::ForwardVector * DistanceToHeadCenter);
}