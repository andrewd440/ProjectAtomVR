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

	/** USceneComponent Interface Begin */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	/** USceneComponent Interface End */

	/** UActorComponent Interface Begin */
public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
protected:
	virtual void PostNetReceive() override;
	virtual void PreNetReceive() override;
	/** UActorComponent Interface End */

private:
	UFUNCTION(Server, WithValidation, Reliable)
	void ServerSendTransform(const FVector_NetQuantize10 Location, const FRotator Rotation);

public:
	/** 
	 * Delegate called after a transform update has been received from the network. 
	 * @param DeltaTime The time since the last net update was received.
	 */
	DECLARE_DELEGATE_OneParam(FPostNetTransformUpdate, float /*DeltaTime*/)
	FPostNetTransformUpdate OnPostNetTransformUpdate;

protected:
	/** Times per second transform updates are sent to the server */
	UPROPERTY(EditDefaultsOnly, Category = NetCamera)
	float NetUpdateFrequency = 50.f;

	float LastNetUpdate = 0.f;
};
