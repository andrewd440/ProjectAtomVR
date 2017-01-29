// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameModes/AtomGameObjective.h"
#include "AtomControlPoint.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API AAtomControlPoint : public AAtomGameObjective
{
	GENERATED_BODY()
		
public:
	AAtomControlPoint();

	UBoxComponent* GetCaptureBounds() const;

	UStaticMeshComponent* GetOutlineMesh() const;

	virtual void InitializeObjective() override;

protected:
	UPROPERTY(BlueprintReadWrite, Transient, Category = ControlPoint)
	AAtomTeamInfo* ControllingTeam = nullptr;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ControlPoint, meta = (AllowPrivateAccess="true"))
	UBoxComponent* CaptureBounds;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ControlPoint, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* OutlineMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ControlPoint, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* InnerSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ControlPoint, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* OuterSphere;
};

FORCEINLINE UBoxComponent* AAtomControlPoint::GetCaptureBounds() const { return CaptureBounds; }

FORCEINLINE UStaticMeshComponent* AAtomControlPoint::GetOutlineMesh() const { return OutlineMesh; }
