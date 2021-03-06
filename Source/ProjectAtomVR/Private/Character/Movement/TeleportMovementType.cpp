// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "TeleportMovementType.h"

#include "Components/SplineMeshComponent.h"
#include "Components/SplineComponent.h"

#include "NetMotionControllerComponent.h"
#include "HMDCameraComponent.h"

UTeleportMovementType::UTeleportMovementType(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	bIsTeleportActive = false;
	bIsTargetValid = false;
	bIsGripPressed = false;

	ArcSpline = CreateDefaultSubobject<USplineComponent>(TEXT("ArcSpline"));
}

void UTeleportMovementType::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	FName TeleportInput, GripInput;
	if (GetCharacter()->IsRightHanded())
	{
		TeleportInput = TEXT("TeleportLeft");
		GripInput = TEXT("GripLeft");
	}
	else
	{
		TeleportInput = TEXT("TeleportRight");
		GripInput = TEXT("GripRight");
	}

	InputComponent->BindAction(GripInput, IE_Pressed, this, &UTeleportMovementType::OnGripPressed);
	InputComponent->BindAction(GripInput, IE_Released, this, &UTeleportMovementType::OnGripReleased);

	InputComponent->BindAction(TeleportInput, IE_Pressed, this, &UTeleportMovementType::OnTeleportPressed);
	InputComponent->BindAction(TeleportInput, IE_Released, this, &UTeleportMovementType::OnTeleportReleased);
}

void UTeleportMovementType::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsTeleportActive)
	{
		FHitResult Hit;
		TArray<FVector> ArcPath;

		bIsTargetValid = false;
		FPredictProjectilePathResult PathResult;
		if (TraceArcPath(Hit, PathResult))
		{		
			UpdateTeleportActor(PathResult.HitResult);
		}

		TeleportActor->SetActorHiddenInGame(!bIsTargetValid);

		UpdateArcSpline(ArcPath);
	}
}

bool UTeleportMovementType::TraceArcPath(FHitResult& OutHit, FPredictProjectilePathResult& PathResult)
{
	FPredictProjectilePathParams PredictParams{ 0.f, ArcSpline->GetComponentLocation(), ArcSpline->GetForwardVector() * TeleportArcVelocity, 2.5f };
	PredictParams.SimFrequency = 7.f;

	return UGameplayStatics::PredictProjectilePath(this, PredictParams, PathResult);
}

void UTeleportMovementType::UpdateArcSpline(const TArray<FVector>& ArcPath)
{
	// Update the spline component with new points
	ArcSpline->ClearSplinePoints();

	for (const auto& Point : ArcPath)
	{
		ArcSpline->AddSplinePoint(Point, ESplineCoordinateSpace::World, false);
	}	

	const int32 ArcPathNum = ArcPath.Num();
	const int32 SplineMeshNum = SplineMeshes.Num();
	const uint32 LastIndex = ArcPathNum - 1;
	ArcSpline->SetSplinePointType(LastIndex, ESplinePointType::CurveClamped);


	//----------------------------------------------------------------------------------------------
	// Create/Update spline meshes using information from spline component
	//----------------------------------------------------------------------------------------------

	// Add/Remove meshes as needed
	const int32 SplineMeshesNeeded = ArcPathNum - 1;
	if (SplineMeshNum < SplineMeshesNeeded)
	{
		// Add more meshes
		const int32 NewMeshesNeeded = SplineMeshesNeeded - SplineMeshNum;
		for (int32 i = 0; i < NewMeshesNeeded; ++i)
		{
			USplineMeshComponent* NewMesh = NewObject<USplineMeshComponent>(this);
			NewMesh->SetMaterial(0, ArcMaterial);
			NewMesh->SetStaticMesh(ArcMesh);
			NewMesh->SetMobility(EComponentMobility::Movable);

			NewMesh->RegisterComponent();
			SplineMeshes.Add(NewMesh);
			
		}
	}
	else if (SplineMeshNum > SplineMeshesNeeded)
	{	
		constexpr int32 MeshBuffer = 4;
		if (SplineMeshNum > SplineMeshesNeeded + MeshBuffer)
		{
			// Delete unneeded meshes if beyond buffer
			for (int32 i = SplineMeshesNeeded; i < SplineMeshNum; ++i)
			{
				SplineMeshes[i]->DestroyComponent();
			}

			SplineMeshes.RemoveAt(SplineMeshesNeeded, SplineMeshNum - SplineMeshesNeeded);
		}
		else
		{
			// Hide unneeded meshes
			for (int32 i = SplineMeshesNeeded; i < SplineMeshNum; ++i)
			{
				SplineMeshes[i]->SetHiddenInGame(true);
			}
		}
	}

	// Create meshes from spline points
	for (int32 i = 0; i < SplineMeshesNeeded; ++i)
	{
		USplineMeshComponent* SplineMesh = SplineMeshes[i];

		SplineMesh->SetHiddenInGame(false);

		FVector StartPos, StartTangent, EndPos, EndTangent;
		ArcSpline->GetLocationAndTangentAtSplinePoint(i, StartPos, StartTangent, ESplineCoordinateSpace::World);
		ArcSpline->GetLocationAndTangentAtSplinePoint(i+1, EndPos, EndTangent, ESplineCoordinateSpace::World);
		SplineMesh->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent);
	}
}

void UTeleportMovementType::UpdateTeleportActor(const FHitResult& DestinationHit)
{
	// Check if the hit was in a valid position
	if (UNavigationSystem* const NavSystem = GetWorld()->GetNavigationSystem())
	{
		FNavLocation NavLocation;
		if (NavSystem->ProjectPointToNavigation(DestinationHit.Location, NavLocation, FVector{ 500.f, 500.f, 500.f }))
		{
			// We are on the navmesh, now we need to trace down to find the actual floor. The navmesh location
			// is floating above the floor a little.
			FHitResult FloorHit;
			FCollisionObjectQueryParams CollisionParams{ ECollisionChannel::ECC_WorldStatic };
			const FVector TraceEnd = NavLocation.Location - FVector::UpVector * 200.f;

			if (GetWorld()->LineTraceSingleByObjectType(FloorHit, NavLocation.Location, TraceEnd, CollisionParams))
			{
				TeleportActor->SetActorLocation(FloorHit.ImpactPoint, false, nullptr, ETeleportType::TeleportPhysics);			
			}
			else
			{
				// If trace went bad, default to original nav hit
				TeleportActor->SetActorLocation(NavLocation.Location, false, nullptr, ETeleportType::TeleportPhysics);
			}

			const float TeleportYaw = GetCharacter()->GetCamera()->GetComponentRotation().Yaw;
			TeleportActor->SetActorRotation(FRotator{ 0, TeleportYaw, 0 });

			bIsTargetValid = true;
		}
	}
}

void UTeleportMovementType::DestroyComponent(bool bPromoteChildren /*= false*/)
{
	if (TeleportActor)
	{
		TeleportActor->Destroy();
		TeleportActor = nullptr;
	}

	if (ArcSpline)
	{
		ArcSpline->DestroyComponent();
		ArcSpline = nullptr;
	}

	for (auto Mesh : SplineMeshes)
	{
		Mesh->DestroyComponent();		
	}

	SplineMeshes.Empty();

	Super::DestroyComponent(bPromoteChildren);
}

void UTeleportMovementType::OnTeleportPressed()
{
	// Create and/or show the teleport actor
	if (!bIsGripPressed)
	{
		if (TeleportActor == nullptr)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Name = TEXT("TeleportActor");
			SpawnParams.Owner = GetOwner();
			TeleportActor = GetWorld()->SpawnActor<AActor>(TeleportActorTemplate ? TeleportActorTemplate : AActor::StaticClass(), SpawnParams);
			TeleportActor->SetReplicates(false);
		}

		// Attach the teleport arc to non-dominate hand
		ArcSpline->AttachToComponent(GetCharacter()->GetHandController<EHandType::Nondominate>(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);

		bIsTeleportActive = true;
	}
}

void UTeleportMovementType::OnTeleportReleased()
{
	if (bIsTargetValid && !bIsGripPressed)
	{
		GetCharacter()->MovementTeleport(TeleportActor->GetActorLocation(), TeleportActor->GetActorRotation());
	}

	bIsTeleportActive = false;

	if (TeleportActor)
	{
		TeleportActor->SetActorHiddenInGame(true);
	}

	if (ArcSpline)
	{
		ArcSpline->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		ArcSpline->ClearSplinePoints();
	}

	for (auto Mesh : SplineMeshes)
	{
		Mesh->DestroyComponent();
	}

	SplineMeshes.Empty();
}

void UTeleportMovementType::OnGripPressed()
{
	bIsGripPressed = true;
}

void UTeleportMovementType::OnGripReleased()
{
	bIsGripPressed = false;
}
