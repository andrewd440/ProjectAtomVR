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

FPrimitiveSceneProxy* UHMDCapsuleComponent::CreateSceneProxy()
{
	// Copied from CapsuleComponent to adjust params.
	// Represents a UCapsuleComponent to the scene manager. 
	class FDrawCylinderSceneProxy : public FPrimitiveSceneProxy
	{
	public:
		FDrawCylinderSceneProxy(const UHMDCapsuleComponent* InComponent)
			: FPrimitiveSceneProxy(InComponent)
			, bDrawOnlyIfSelected(InComponent->bDrawOnlyIfSelected)
			, CapsuleRadius(InComponent->CapsuleRadius)
			, CapsuleHalfHeight(InComponent->CapsuleHalfHeight)
			, ShapeColor(InComponent->ShapeColor)
		{
			bWillEverBeLit = false;
		}

		virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
		{
			QUICK_SCOPE_CYCLE_COUNTER(STAT_GetDynamicMeshElements_DrawDynamicElements);


			const FMatrix& LocalToWorld = GetLocalToWorld();
			const int32 CapsuleSides = FMath::Clamp<int32>(CapsuleRadius / 4.f, 16, 64);

			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{

				if (VisibilityMap & (1 << ViewIndex))
				{
					const FSceneView* View = Views[ViewIndex];
					const FLinearColor DrawCapsuleColor = GetViewSelectionColor(ShapeColor, *View, IsSelected(), IsHovered(), false, IsIndividuallySelected());

					FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);
					DrawWireCapsule(PDI, LocalToWorld.GetOrigin(), LocalToWorld.GetScaledAxis(EAxis::X), LocalToWorld.GetScaledAxis(EAxis::Y), LocalToWorld.GetScaledAxis(EAxis::Z), DrawCapsuleColor, CapsuleRadius, CapsuleHalfHeight, CapsuleSides, SDPG_World, 1.25f);
				}
			}
		}

		virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
		{
			const bool bProxyVisible = !bDrawOnlyIfSelected || IsSelected();

			// Should we draw this because collision drawing is enabled, and we have collision
			const bool bShowForCollision = View->Family->EngineShowFlags.Collision && IsCollisionEnabled();

			FPrimitiveViewRelevance Result;
			Result.bDrawRelevance = (IsShown(View) && bProxyVisible) || bShowForCollision;
			Result.bDynamicRelevance = true;
			Result.bShadowRelevance = IsShadowCast(View);
			Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
			return Result;
		}
		virtual uint32 GetMemoryFootprint(void) const override { return(sizeof(*this) + GetAllocatedSize()); }
		uint32 GetAllocatedSize(void) const { return(FPrimitiveSceneProxy::GetAllocatedSize()); }

	private:
		const uint32	bDrawOnlyIfSelected : 1;
		const float		CapsuleRadius;
		const float		CapsuleHalfHeight;
		const FColor	ShapeColor;
	};

	return new FDrawCylinderSceneProxy(this);
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
	FTransform CollisionTransform = ComponentToWorld;
	CollisionTransform.AddToTranslation(CollisionOffset);

	return CollisionTransform.ToMatrixNoScale();
}

FBoxSphereBounds UHMDCapsuleComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	FBoxSphereBounds NewBounds = Super::CalcBounds(LocalToWorld);
	NewBounds.Origin += CollisionOffset;
	return NewBounds;
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
	const FVector HeadCenter = Camera->GetRelativeHeadLocation();

	// First sweep to the new location to determine if we need to move the component location to offset
	// any new collision from moving the HMD.
	{
		const FVector PendingCollisionOffset{ HeadCenter.X, HeadCenter.Y, GetUnscaledCapsuleHalfHeight() };

		const FVector WorldLocation = GetComponentLocation();
		const FVector Start = WorldLocation + CollisionOffset;
		const FVector End = WorldLocation + PendingCollisionOffset;

		FCollisionQueryParams QueryParams{ NAME_None, false, GetOwner() };
		FCollisionResponseParams ResponseParam;
		InitSweepCollisionParams(QueryParams, ResponseParam);

		const ECollisionChannel TraceChannel = GetCollisionObjectType();
		FHitResult SweepHit{ 1.f };
		FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(GetScaledCapsuleRadius(), GetScaledCapsuleHalfHeight());

		if (GetWorld()->SweepSingleByChannel(SweepHit, Start, End, FQuat::Identity, TraceChannel, CapsuleShape, QueryParams, ResponseParam))
		{
			// We hit a surface. Move the component opposite of the hit.
			FVector Delta = End - SweepHit.Location;
			FVector MoveDelta = FVector::DotProduct(Delta, -SweepHit.Normal) * SweepHit.Normal;
			MoveDelta *= 1.f + THRESH_POINT_ON_PLANE; // Add a little to prevent penetration
			MoveComponent(MoveDelta, GetComponentQuat(), false);
		}
	}

	// Adjust capsule height if needed.
	constexpr float MaxCapsuleHeightError = 5.0f; // Max difference between HMD height and capsule height allowed
	constexpr float CapsuleHeightPadding = 25.f;  // Height padding applied to capsule in addition to HMD height
	const float PerfectCapsuleHalfHeight = (HeadCenter.Z + CapsuleHeightPadding) / 2.f;
	if (FMath::Abs(PerfectCapsuleHalfHeight - GetUnscaledCapsuleHalfHeight()) > MaxCapsuleHeightError)
	{
		CapsuleHalfHeight = PerfectCapsuleHalfHeight;
	}	

	// Center capsule on player head in XY and place base at negative HMD height
	CollisionOffset = FVector{ HeadCenter.X, HeadCenter.Y, GetUnscaledCapsuleHalfHeight() };;

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
