#pragma once

#include "Engine.h"
#include "AtomTypes.generated.h"

UENUM()
enum class EHand : uint8
{
	Left,
	Right
};

UENUM()
enum class EHandType : uint8
{
	Dominate,
	Nondominate
};

USTRUCT()
struct PROJECTATOMVR_API FMotionTransformRep
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FVector_NetQuantize10 Location;
	
	UPROPERTY()
	FRotator Rotation;
};

//----------------------------------------------------------------------------------------------
// Scope based object used to track movement changes in one component and transfer them to
// another component. The movement component will then be reset to it's original location and 
// rotation.
//----------------------------------------------------------------------------------------------
class FScopedMovementTransfer
{
public:
	FScopedMovementTransfer(class USceneComponent& UpdatedComponent, class USceneComponent& TransferComponent);
	~FScopedMovementTransfer();

private:
	class USceneComponent* UpdatedComponent;  // The component that is being moved
	class USceneComponent* TransferComponent; // The compoennt to transfer moves to

	// Original location and rotation of UpdateComponent
	FVector StartWorldLocation;
	FVector StartRelativeLocation;

	FQuat StartWorldRotation;
	FQuat StartRelativeRotation;
};