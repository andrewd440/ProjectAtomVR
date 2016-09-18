// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "HeroMovementType.h"
#include "TeleportMovementType.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UTeleportMovementType : public UHeroMovementType
{
	GENERATED_BODY()
	
public:
	UTeleportMovementType(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());


	/** UHeroMovementType Interface Begin */
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	/** UHeroMovementType Interface End */

	/** UActorComponent Interface Begin */
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void DestroyComponent(bool bPromoteChildren = false) override;
	/** UActorComponent Interface End */

protected:
	virtual void OnTeleportPressed();

	virtual void OnTeleportReleased();

protected:
	// Actor template that will represent the end position of a teleport
	UPROPERTY(EditDefaultsOnly, Category = HeroTeleport)
	TSubclassOf<AActor> TeleportActorTemplate;

	/** Material to use for the teleport arc */
	UPROPERTY(EditAnywhere, Category = HeroTeleport)
	UMaterial* ArcMaterial;

	/** Mesh to use for the teleport arc */
	UPROPERTY(EditAnywhere, Category = HeroTeleport)
	UStaticMesh* ArcMesh;

	// Used to show arc trajectory of teleport
	UPROPERTY()
	class USplineComponent* ArcSpline = nullptr;

	uint32 bIsTeleportActive : 1;

	// Actor used represent the end position of a teleport
	AActor* TeleportActor = nullptr;
};
