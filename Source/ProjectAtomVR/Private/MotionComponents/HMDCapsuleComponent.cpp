// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "HMDCapsuleComponent.h"
#include "HMDCameraComponent.h"

#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/SphylElem.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"
#include "Engine/EngineTypes.h"
#include "WorldCollision.h"

DEFINE_LOG_CATEGORY_STATIC(LogHMDCapsule, Log, All);

UHMDCapsuleComponent::UHMDCapsuleComponent(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PrePhysics;

	bAbsoluteLocation = true;
	bAbsoluteRotation = true;

	bUseArchetypeBodySetup = false;
	bWantsInitializeComponent = true;
}

void UHMDCapsuleComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(Camera)
	{
		UpdateCollisionOffset();
	}
}

FMatrix UHMDCapsuleComponent::GetRenderMatrix() const
{
	const FVector LocalOffset = RelativeRotation.RotateVector(CollisionOffset);
	FTransform CollisionTransform = ComponentToWorld;	
	CollisionTransform.AddToTranslation(LocalOffset);

	return CollisionTransform.ToMatrixNoScale();
}

FBoxSphereBounds UHMDCapsuleComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	FVector BoxPoint = FVector(CapsuleRadius, CapsuleRadius, CapsuleHalfHeight);
	return FBoxSphereBounds(CollisionOffset, BoxPoint, CapsuleHalfHeight).TransformBy(LocalToWorld);
}

void UHMDCapsuleComponent::UpdateBodySetup()
{
	bUseArchetypeBodySetup = false;

	Super::UpdateBodySetup();

	FKSphylElem* SE = ShapeBodySetup->AggGeom.SphylElems.GetData();
	SE->Center = CollisionOffset;
}

void UHMDCapsuleComponent::InitializeComponent()
{
	Super::InitializeComponent();

	AActor* const Owner = GetOwner();
	Camera = Owner->FindComponentByClass<UHMDCameraComponent>();
}

void UHMDCapsuleComponent::UpdateCollisionOffset()
{
	const FVector WorldHeadCenter = Camera->GetWorldHeadLocation();	
	const FVector RelativeHeadCenter = ComponentToWorld.InverseTransformPosition(WorldHeadCenter);

	// First sweep to the new location to determine if we need to move the component location to offset
	// any new collision from moving the HMD.
	if(GetOwner()->Role > ENetRole::ROLE_SimulatedProxy)
	{
		const FVector PendingCollisionOffset{ RelativeHeadCenter.X, RelativeHeadCenter.Y, GetUnscaledCapsuleHalfHeight() };

		FVector Start = ComponentToWorld.TransformPosition(CollisionOffset);
		FVector End = ComponentToWorld.TransformPosition(PendingCollisionOffset);

		FCollisionQueryParams QueryParams{ NAME_None, false, GetOwner() };
		FCollisionResponseParams ResponseParam;
		InitSweepCollisionParams(QueryParams, ResponseParam);

		const ECollisionChannel TraceChannel = GetCollisionObjectType();
		FHitResult SweepHit{ 1.f };
		FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(GetScaledCapsuleRadius(), GetScaledCapsuleHalfHeight());

		// #AtomTodo Need to find a proper way to replicate this to prevent/work with server correction
		if (GetWorld()->SweepSingleByChannel(SweepHit, Start, End, FQuat::Identity, TraceChannel, CapsuleShape, QueryParams, ResponseParam))
		{
			// We hit a surface. Move the component opposite of the hit.
			const FVector RemainingDelta = End - SweepHit.Location;
			FVector MoveDelta = FVector::DotProduct(RemainingDelta, -SweepHit.Normal) * SweepHit.Normal;
			MoveDelta *= 1.f + THRESH_POINT_ON_PLANE; // Add a little to prevent penetration
			MoveComponent(MoveDelta, GetComponentQuat(), false);

			// Now sweep using the remaining movement that was not canceled out by MoveDelta.
			Start = SweepHit.Location + SweepHit.Normal * THRESH_POINT_ON_PLANE;
			End += MoveDelta;
			if (GetWorld()->SweepSingleByChannel(SweepHit, Start, End, FQuat::Identity, TraceChannel, CapsuleShape, QueryParams, ResponseParam))
			{
				// We hit something else, just move to the hit location, with a small offset to prevent penetration
				MoveDelta = (SweepHit.Location - End) * (1.f + THRESH_POINT_ON_PLANE);
				MoveComponent(MoveDelta, GetComponentQuat(), false);
			}
		}
	}

	// Adjust capsule height if needed.
	constexpr float MaxCapsuleHeightError = 5.0f; // Max difference between HMD height and capsule height allowed
	constexpr float CapsuleHeightPadding = 25.f;  // Height padding applied to capsule in addition to HMD height
	const float PerfectCapsuleHalfHeight = (RelativeHeadCenter.Z + CapsuleHeightPadding) / 2.f;
	if (FMath::Abs(PerfectCapsuleHalfHeight - GetUnscaledCapsuleHalfHeight()) > MaxCapsuleHeightError)
	{
		CapsuleHalfHeight = PerfectCapsuleHalfHeight;
	}	

	// Center capsule on player head in XY and place base at negative HMD height
	CollisionOffset = FVector{ RelativeHeadCenter.X, RelativeHeadCenter.Y, GetUnscaledCapsuleHalfHeight() };;

	// Update collision
	UpdateBounds();
	UpdateBodySetup();
	MarkRenderStateDirty();

	// do this if already created
	// otherwise, it hasn't been really created yet
	if (bPhysicsStateCreated)
	{
		// Update physics engine collision shapes
		BodyInstance.UpdateBodyScale(ComponentToWorld.GetScale3D(), true);

		if (IsCollisionEnabled() && GetOwner())
		{
			UpdateOverlaps();
		}
	}
}

bool UHMDCapsuleComponent::MoveComponentImpl(const FVector& Delta, const FQuat& NewRotationQuat, bool bSweep, FHitResult* OutHit /*= NULL*/, EMoveComponentFlags MoveFlags /*= MOVECOMP_NoFlags*/, ETeleportType Teleport /*= ETeleportType::None*/)
{
	const bool bResult = Super::MoveComponentImpl(Delta, NewRotationQuat, bSweep, OutHit, MoveFlags, Teleport);
	return bResult;
}
