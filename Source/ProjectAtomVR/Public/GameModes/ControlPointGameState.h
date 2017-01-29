// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameModes/AtomGameState.h"
#include "ControlPointGameState.generated.h"

class AAtomControlPoint;

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API AControlPointGameState : public AAtomGameState
{
	GENERATED_BODY()
	
public:
	AControlPointGameState();
	
	void SetActiveControlPoint(AAtomControlPoint* ControlPoint);

	AAtomControlPoint* GetActiveControlPoint() const;

	/** AtomGameState Interface Begin */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	/** AtomGameState Interface End */	

protected:
	UPROPERTY(Replicated, Transient, BlueprintReadOnly, Category = ControlPointGameState)
	AAtomControlPoint* ActiveControlPoint = nullptr;
};
