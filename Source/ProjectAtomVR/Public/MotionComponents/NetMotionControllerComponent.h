// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "MotionControllerComponent.h"
#include "NetMotionControllerComponent.generated.h"

/**
 * Motion Controller Component that supports movement replication.
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class PROJECTATOMVR_API UNetMotionControllerComponent : public UMotionControllerComponent
{
	GENERATED_BODY()
	
public:
	/** UMotionControllerComponent Interface Begin */
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	/** UMotionControllerComponent Interface End */

	/** USceneComponent Interface Begin */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	/** USceneComponent Interface End */

private:
	UFUNCTION(Server, WithValidation, Reliable)
	void ServerSendTransform(const FVector_NetQuantize10 Location, const FRotator Rotation);

protected:
	/** Times per second transform updates are sent to the server */
	UPROPERTY(EditDefaultsOnly, Category = NetMotionController)
	float NetUpdateFrequency = 30.f;

	float LastNetUpdate = 0.f;
};
