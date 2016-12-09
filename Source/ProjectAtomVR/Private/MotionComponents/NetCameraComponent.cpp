// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "NetCameraComponent.h"

namespace
{
	static FVector SavedLocation = FVector::ZeroVector;
	static FRotator SavedRotation = FRotator::ZeroRotator;
}

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

	const float CurrentTime = GetWorld()->GetRealTimeSeconds();
	const float DeltaTime = CurrentTime - LastNetUpdate;

	// Prevent doubling transform updates. If delta time is zero, wait for next one to invoke call.
	if (DeltaTime > 0.f)
	{
		OnPostNetTransformUpdate.ExecuteIfBound(CurrentTime - LastNetUpdate);
		LastNetUpdate = CurrentTime;
	}
}

bool UNetCameraComponent::ServerSendTransform_Validate(const FVector_NetQuantize10 Location, const FRotator Rotation)
{
	return true;
}

void UNetCameraComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	bool bIsControlled = false;

	if (IsInGameThread())
	{
		const APawn* MyPawn = Cast<APawn>(GetOwner());
		bIsControlled = MyPawn ? MyPawn->IsLocallyControlled() : false;
	}

	if (bIsControlled && GetIsReplicated() && !GetOwner()->HasAuthority())
	{
		const float CurrentTime = GetWorld()->GetRealTimeSeconds();
		if (CurrentTime - LastNetUpdate > 1.f / NetUpdateFrequency)
		{			
			ServerSendTransform(RelativeLocation, RelativeRotation);
			LastNetUpdate = CurrentTime;
		}
	}
}

void UNetCameraComponent::PostNetReceive()
{
	Super::PostNetReceive();

	// Check if transform was updated
	if (SavedLocation != RelativeLocation || SavedRotation != RelativeRotation)
	{
		const float CurrentTime = GetWorld()->GetRealTimeSeconds();
		OnPostNetTransformUpdate.ExecuteIfBound(CurrentTime - LastNetUpdate);
		LastNetUpdate = CurrentTime;
	}
}

void UNetCameraComponent::PreNetReceive()
{
	Super::PreNetReceive();

	SavedLocation = RelativeLocation;
	SavedRotation = RelativeRotation;
}

void UNetCameraComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Only send transforms to simulated proxies
	DOREPLIFETIME_CHANGE_CONDITION(USceneComponent, RelativeLocation, COND_SimulatedOnly);
	DOREPLIFETIME_CHANGE_CONDITION(USceneComponent, RelativeRotation, COND_SimulatedOnly);
	DOREPLIFETIME_CHANGE_CONDITION(USceneComponent, RelativeScale3D, COND_SimulatedOnly);
}
