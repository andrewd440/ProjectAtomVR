// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "NetMotionControllerComponent.h"


void UNetMotionControllerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UNetMotionControllerComponent::ServerSendTransform_Implementation(const FVector_NetQuantize10 Location, const FRotator Rotation)
{
	SetRelativeLocationAndRotation(Location, Rotation);
}

bool UNetMotionControllerComponent::ServerSendTransform_Validate(const FVector_NetQuantize10 Location, const FRotator Rotation)
{
	return true;
}

void UNetMotionControllerComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsTracked() && GetIsReplicated() && !GetOwner()->HasAuthority())
	{
		LastNetUpdate += DeltaTime;

		if (LastNetUpdate > 1.f / NetUpdateFrequency)
		{
			LastNetUpdate = 0.f;
			ServerSendTransform(RelativeLocation, RelativeRotation);
		}
	}
}


