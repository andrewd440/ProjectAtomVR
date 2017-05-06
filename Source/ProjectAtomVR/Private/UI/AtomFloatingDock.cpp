// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomFloatingDock.h"

AAtomFloatingDock::AAtomFloatingDock()
{
	if (UNLIKELY(IsRunningDedicatedServer()))
	{
		return;
	}

	// Create root default scene component
	{
		SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
		check(SceneComponent != nullptr);

		RootComponent = SceneComponent;
	}

	UStaticMesh* LineSegmentCylinderMesh = nullptr;
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> ObjectFinder(TEXT("/Game/UI/FloatingDock/LineSegmentCylinder"));
		LineSegmentCylinderMesh = ObjectFinder.Object;
		check(LineSegmentCylinderMesh != nullptr);
	}

	UStaticMesh* JointSphereMesh = nullptr;
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> ObjectFinder(TEXT("/Game/UI/FloatingDock/JointSphere"));
		JointSphereMesh = ObjectFinder.Object;
		check(JointSphereMesh != nullptr);
	}

	{
		static ConstructorHelpers::FObjectFinder<UMaterial> ObjectFinder(TEXT("/Game/UI/FloatingDock/LineMaterial"));
		LineMaterial = ObjectFinder.Object;
		check(LineMaterial != nullptr);
	}



	constexpr bool bAllowTextLighting = false;

	{
		FirstLineComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FirstLine"));
		check(FirstLineComponent != nullptr);

		FirstLineComponent->SetStaticMesh(LineSegmentCylinderMesh);
		FirstLineComponent->SetMobility(EComponentMobility::Movable);
		FirstLineComponent->SetupAttachment(SceneComponent);

		FirstLineComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		FirstLineComponent->bGenerateOverlapEvents = false;
		FirstLineComponent->SetCanEverAffectNavigation(false);
		FirstLineComponent->bCastDynamicShadow = bAllowTextLighting;
		FirstLineComponent->bCastStaticShadow = false;
		FirstLineComponent->bAffectDistanceFieldLighting = bAllowTextLighting;
		FirstLineComponent->bAffectDynamicIndirectLighting = bAllowTextLighting;
	}

	{
		JointSphereComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("JointSphere"));
		check(JointSphereComponent != nullptr);

		JointSphereComponent->SetStaticMesh(JointSphereMesh);
		JointSphereComponent->SetMobility(EComponentMobility::Movable);
		JointSphereComponent->SetupAttachment(SceneComponent);

		JointSphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		JointSphereComponent->bGenerateOverlapEvents = false;
		JointSphereComponent->SetCanEverAffectNavigation(false);
		JointSphereComponent->bCastDynamicShadow = bAllowTextLighting;
		JointSphereComponent->bCastStaticShadow = false;
		JointSphereComponent->bAffectDistanceFieldLighting = bAllowTextLighting;
		JointSphereComponent->bAffectDynamicIndirectLighting = bAllowTextLighting;

	}

	{
		SecondLineComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SecondLine"));
		check(SecondLineComponent != nullptr);

		SecondLineComponent->SetStaticMesh(LineSegmentCylinderMesh);
		SecondLineComponent->SetMobility(EComponentMobility::Movable);
		SecondLineComponent->SetupAttachment(SceneComponent);

		SecondLineComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		SecondLineComponent->bGenerateOverlapEvents = false;
		SecondLineComponent->SetCanEverAffectNavigation(false);
		SecondLineComponent->bCastDynamicShadow = bAllowTextLighting;
		SecondLineComponent->bCastStaticShadow = false;
		SecondLineComponent->bAffectDistanceFieldLighting = bAllowTextLighting;
		SecondLineComponent->bAffectDynamicIndirectLighting = bAllowTextLighting;

	}
}


void AAtomFloatingDock::PostActorCreated()
{
	Super::PostActorCreated();

	// Create an MID so that we can change parameters on the fly (fading)
	check(LineMaterial != nullptr);
	LineMaterialMID = UMaterialInstanceDynamic::Create(LineMaterial, this);

	FirstLineComponent->SetMaterial(0, LineMaterialMID);
	JointSphereComponent->SetMaterial(0, LineMaterialMID);
	SecondLineComponent->SetMaterial(0, LineMaterialMID);
}


void AAtomFloatingDock::SetOpacity(const float NewOpacity)
{
	const FLinearColor NewColor = FLinearColor(0.6f, 0.6f, 0.6f).CopyWithNewOpacity(NewOpacity);	// #AtomTodo Tweak brightness
	const FColor NewFColor = NewColor.ToFColor(false);

	check(LineMaterialMID != nullptr);
	static FName ColorAndOpacityParameterName("ColorAndOpacity");
	LineMaterialMID->SetVectorParameterValue(ColorAndOpacityParameterName, NewColor);
}


void AAtomFloatingDock::SetLineRadius(const float Radius)
{
	LineRadius = Radius;
}

void AAtomFloatingDock::SetFirstLineLength(const float Length)
{
	FirstLineLength = Length;
}

void AAtomFloatingDock::Update(const FVector OrientateToward)
{
	// Orientate it toward the viewer
	const FVector DirectionToward = (OrientateToward - GetActorLocation()).GetSafeNormal();
	const FQuat TowardRotation = DirectionToward.ToOrientationQuat();

	// NOTE: The origin of the actor will be the designated target of the text
	const FVector FirstLineLocation = FVector::ZeroVector;
	const FQuat FirstLineRotation = FVector::ForwardVector.ToOrientationQuat();
	const FVector FirstLineScale = FVector(FirstLineLength, LineRadius, LineRadius);
	FirstLineComponent->SetRelativeLocation(FirstLineLocation);
	FirstLineComponent->SetRelativeRotation(FirstLineRotation);
	FirstLineComponent->SetRelativeScale3D(FirstLineScale);

	// NOTE: The joint sphere draws at the connection point between the lines
	const FVector JointLocation = FirstLineLocation + FirstLineRotation * FVector::ForwardVector * FirstLineLength;
	const FVector JointScale = FVector(LineRadius);
	JointSphereComponent->SetRelativeLocation(JointLocation);
	JointSphereComponent->SetRelativeScale3D(JointScale);

	// NOTE: The second line starts at the joint location
	SecondLineComponent->SetWorldLocation(JointSphereComponent->GetComponentLocation());
	SecondLineComponent->SetWorldRotation((TowardRotation * -FVector::RightVector).ToOrientationQuat());	

	UpdateInternal(OrientateToward, TowardRotation);
}

void AAtomFloatingDock::UpdateInternal(const FVector& OrientateToward, const FQuat& TowardRotation)
{
	// Do nothing
}

void AAtomFloatingDock::SetSecondLineLength(const float Length)
{
	SecondLineComponent->SetRelativeScale3D(FVector{
		(Length / GetActorScale().X) * GetWorld()->GetWorldSettings()->WorldToMeters / 100.0f, 
		LineRadius, 
		LineRadius });
}
