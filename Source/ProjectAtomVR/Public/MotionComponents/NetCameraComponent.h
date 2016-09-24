// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Camera/CameraComponent.h"
#include "NetCameraComponent.generated.h"

/**
 * Camera Component that supports movement replication.
 */
UCLASS()
class PROJECTATOMVR_API UNetCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

public:
	UNetCameraComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** UActorComponent Interface Begin */
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	/** UActorComponent Interface End */

private:
	UFUNCTION(Server, WithValidation, Reliable)
	void ServerSendTransform(const FVector_NetQuantize10 Location, const FRotator Rotation);

protected:
	/** Times per second transform updates are sent to the server */
	UPROPERTY(EditDefaultsOnly, Category = NetMotionController)
	float NetUpdateFrequency = 30.f;

	float LastNetUpdate = 0.f;
};
