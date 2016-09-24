#pragma once

#include "Engine.h"
#include "AtomTypes.generated.h"

USTRUCT()
struct PROJECTATOMVR_API FMotionTransformRep
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FVector_NetQuantize10 Location;
	
	UPROPERTY()
	FRotator Rotation;
};