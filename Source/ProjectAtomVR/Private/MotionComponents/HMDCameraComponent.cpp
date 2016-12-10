// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "HMDCameraComponent.h"


namespace
{
	constexpr float DistanceToHeadCenter = 17.f;  // ~ distance from the HMD to the center of the players head
	constexpr float DistanceToNeckBase = 15.f; // ~ distance from the head center to the base of the neck

	namespace NeckRangeOfMotion
	{
		constexpr float Pitch = 70.f;
		constexpr float Roll = 45.f;
	}	
}

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

bool UHMDCameraComponent::CalculateTorsoPitchAndRoll(FRotator& TorsoPitchRollOut) const
{
	// Get 2D and 3D directions for forward and right
	const FVector Forward = GetForwardVector();	
	FVector Forward2D = Forward.GetSafeNormal2D();

	const FVector Right = GetRightVector();
	const FVector Right2D = Right.GetSafeNormal2D();
	const float RightDot = FVector::DotProduct(Right, Right2D);

	bool bIsForwardInverted = false;

	// If the up direction is pointing down and right is somewhat close the the right 2D vector, invert forward. Checking the
	// right vector ensures that forward is not inverted if Roll is > +90 degress.
	if (FVector::DotProduct(GetUpVector(), FVector::UpVector) < 0 && 
		RightDot > PI / 4.f)
	{
		Forward2D *= -1.f;	
		bIsForwardInverted = true;
	}

	// Base pitch off forward directions
	float Pitch = FMath::Acos(FVector::DotProduct(Forward, Forward2D)) * (180.f / PI);

	if (Forward.Z < 0)
	{
		Pitch *= -1.f;
	}

	if (Pitch > NeckRangeOfMotion::Pitch)
	{
		TorsoPitchRollOut.Pitch = Pitch - NeckRangeOfMotion::Pitch;
	}
	else if (Pitch < -NeckRangeOfMotion::Pitch)
	{
		TorsoPitchRollOut.Pitch = Pitch + NeckRangeOfMotion::Pitch;
	}	
	
	// Base roll of right directions
	float Roll = FMath::Acos(RightDot) * (180.f / PI);

	if (Right.Z > 0)
	{
		Roll *= -1.f;
	}

	if (Roll > NeckRangeOfMotion::Roll)
	{
		TorsoPitchRollOut.Roll = Roll - NeckRangeOfMotion::Roll;
	}
	else if (Roll < -NeckRangeOfMotion::Roll)
	{
		TorsoPitchRollOut.Roll = Roll + NeckRangeOfMotion::Roll;
	}

	return bIsForwardInverted;
}