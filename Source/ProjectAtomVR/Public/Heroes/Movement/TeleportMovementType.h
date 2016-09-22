// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "HeroMovementType.h"
#include "TeleportMovementType.generated.h"

/**
 * Hero movement type that enables teleportation based movement.
 */
UCLASS(ClassGroup = (Hero), meta = (BlueprintSpawnableComponent))
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

	/**
	* Traces out the current teleportation arc.
	* 
	* @param OutHit		The hit destination of the teleportation.
	* @param OutArcPath	The arc path for the teleportation.
	* @returns True if the path hit something.
	*/
	virtual bool TraceArcPath(FHitResult& OutHit, TArray<FVector>& OutArcPath);

	/**
	* Updates the arc spline that represents the teleportation path.
	* 
	* @param ArcPath The teleportation arc path.
	*/
	virtual void UpdateArcSpline(const TArray<FVector>& ArcPath);

	/**
	* Updates the teleport actor using a destination hit result. Responsible for
	* validating the hit location and setting the bIsTargetValid flag on success.
	* 
	* @param DestinationHit The teleport destination.
	*/
	virtual void UpdateTeleportActor(const FHitResult& DestinationHit);

protected:
	/** Actor template that will represent the end position of a teleport */
	UPROPERTY(EditDefaultsOnly, Category = HeroTeleport)
	TSubclassOf<AActor> TeleportActorTemplate = AActor::StaticClass();

	/** Determines the range of the teleport arc */
	UPROPERTY(EditAnywhere, Category = HeroTeleport)
	float TeleportArcVelocity = 100.f;

	/** Material to use for the teleport arc */
	UPROPERTY(EditAnywhere, Category = HeroTeleport)
	UMaterial* ArcMaterial = nullptr;

	/** Mesh to use for the teleport arc */
	UPROPERTY(EditAnywhere, Category = HeroTeleport)
	UStaticMesh* ArcMesh = nullptr;

	/** If the teleport destination is valid */
	uint32 bIsTargetValid : 1;

	/** Spline meshes used to render the teleport arc each frame. */
	TArray<class USplineMeshComponent*> SplineMeshes;

private:
	uint32 bIsTeleportActive : 1;

	// Used to show arc trajectory of teleport
	UPROPERTY()
	class USplineComponent* ArcSpline = nullptr;

	// Actor used represent the end position of a teleport
	AActor* TeleportActor = nullptr;
};
