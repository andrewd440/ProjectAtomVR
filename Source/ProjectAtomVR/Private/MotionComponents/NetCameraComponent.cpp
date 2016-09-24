// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "NetCameraComponent.h"


UNetCameraComponent::UNetCameraComponent(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UNetCameraComponent::ServerSendTransform_Implementation(const FVector_NetQuantize10 Location, const FRotator Rotation)
{
	SetRelativeLocationAndRotation(Location, Rotation);
}

bool UNetCameraComponent::ServerSendTransform_Validate(const FVector_NetQuantize10 Location, const FRotator Rotation)
{
	return true;
}

void UNetCameraComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	bool bHasAuthority = false;

	if (IsInGameThread())
	{
		const APawn* MyPawn = Cast<APawn>(GetOwner());
		bHasAuthority = MyPawn ? MyPawn->IsLocallyControlled() : false;
	}

	if (bHasAuthority && GetIsReplicated())
	{
		LastNetUpdate += DeltaTime;

		if (LastNetUpdate > 1.f / NetUpdateFrequency)
		{
			LastNetUpdate = 0.f;
			ServerSendTransform(RelativeLocation, RelativeRotation);
		}
	}
}
