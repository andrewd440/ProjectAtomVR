
#include "ProjectAtomVR.h"

FScopedMovementTransfer::FScopedMovementTransfer(USceneComponent& InUpdatedComponent, USceneComponent& InTransferComponent)
	: UpdatedComponent(&InUpdatedComponent)
	, TransferComponent(&InTransferComponent)
{
	StartRelativeLocation = UpdatedComponent->RelativeLocation;
	StartRelativeRotation = UpdatedComponent->RelativeRotation.Quaternion();

	StartWorldLocation = UpdatedComponent->GetComponentLocation();
	StartWorldRotation = UpdatedComponent->GetComponentQuat();
}

FScopedMovementTransfer::~FScopedMovementTransfer()
{
	const FVector LocationDelta = UpdatedComponent->GetComponentLocation() - StartWorldLocation;
	const FQuat RotationDelta = UpdatedComponent->GetComponentQuat() * StartWorldRotation.Inverse();

	TransferComponent->MoveComponent(LocationDelta, TransferComponent->GetComponentQuat() * RotationDelta, false);
	UpdatedComponent->SetRelativeLocationAndRotation(StartRelativeLocation, StartRelativeRotation, false, nullptr, ETeleportType::TeleportPhysics);
}
