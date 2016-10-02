// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "HMDCapsuleComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogHMDCapsule, Log, All);

UHMDCapsuleComponent::UHMDCapsuleComponent(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PrePhysics; // Needed to updated capsule size to match HMD height before physics
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
	if(USceneComponent* const Parent = GetAttachParent())
	{
		constexpr float MaxCapsuleHeightError = 5.0f; // Max difference between HMD height and capsule height allowed
		constexpr float CapsuleHeightPadding = 25.f;  // Height padding applied to capsule in addition to HMD height
		constexpr float DistanceToHeadCenter = 17.f;  // ~ distance from the HMD to the center of the players head

		const FVector CameraLocation = Parent->RelativeLocation;
		const FVector VectorToHead = -Parent->RelativeRotation.Vector() * DistanceToHeadCenter;
		const FVector HeadCenter = CameraLocation + VectorToHead;

		const float PerfectCapsuleHalfHeight = (HeadCenter.Z + CapsuleHeightPadding) / 2.f;
		if (FMath::Abs(PerfectCapsuleHalfHeight - GetUnscaledCapsuleHalfHeight()) > MaxCapsuleHeightError)
		{
			SetCapsuleHalfHeight(PerfectCapsuleHalfHeight);
		}

		// Center capsule on player head in XY and place base at negative HMD height
		FVector NewCapsuleLocation = Parent->GetComponentLocation() + FVector{ VectorToHead.X, VectorToHead.Y, 0.f };
		NewCapsuleLocation.Z += GetUnscaledCapsuleHalfHeight() - Parent->RelativeLocation.Z;
		SetWorldLocation(NewCapsuleLocation);
	}
}

void UHMDCapsuleComponent::OnAttachmentChanged()
{
	Super::OnAttachmentChanged();
	
#if (UE_BUILD_DEBUG | UE_BUILD_DEVELOPMENT)
	if (GetAttachParent() && Cast<UCameraComponent>(GetAttachParent()) == nullptr)
	{
		UE_LOG(LogHMDCapsule, Warning, TEXT("UHMDCapsuleComponent should only be attached to a CameraComponent not %s"), *GetAttachParent()->GetName());
	}
#endif
}
