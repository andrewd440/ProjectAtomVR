// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomFloatingDock.h"

AAtomFloatingDock::AAtomFloatingDock()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bReplicates = false;
	SetActorEnableCollision(false);

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

		FirstLineComponent->SetHiddenInGame(true);
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

		JointSphereComponent->SetHiddenInGame(true);
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

		SecondLineComponent->SetHiddenInGame(true);
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

	bIsActive = false;
	bIsRetracted = true;
	bIsExtended = false;
	FirstLineComponent->SetRelativeScale3D(FVector{ 0.f, LineRadius, LineRadius });
	SecondLineComponent->SetRelativeScale3D(FVector{ 10.f, LineRadius, LineRadius });
	JointSphereComponent->SetRelativeScale3D(FVector{ LineRadius });
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


void AAtomFloatingDock::SetExtensionSpeed(const float Speed)
{
	ExtensionSpeed = Speed;
}

void AAtomFloatingDock::SetLineRadius(const float Radius)
{
	LineRadius = Radius;

	FirstLineComponent->SetRelativeScale3D(FVector{ FirstLineComponent->RelativeScale3D.X, Radius, Radius });
	SecondLineComponent->SetRelativeScale3D(FVector{ SecondLineComponent->RelativeScale3D.X, Radius, Radius });
	JointSphereComponent->SetRelativeScale3D(FVector{ Radius });
}

void AAtomFloatingDock::SetFirstLineLength(const float Length)
{
	FirstLineLength = Length;

	FirstLineComponent->SetRelativeScale3D(FVector{
		(Length / GetActorScale().X) * GetWorld()->GetWorldSettings()->WorldToMeters / 100.0f,
		LineRadius,
		LineRadius });
}

void AAtomFloatingDock::Activate()
{
	if (DeactivationTimerHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(DeactivationTimerHandle);
	}

	if (bIsRetracted)
	{
		FirstLineComponent->SetHiddenInGame(false);
		FirstLineComponent->SetWorldScale3D(FVector{ 0.f, LineRadius, LineRadius });
	}
	
	bIsRetracted = false;
	bIsActive = true;
}

void AAtomFloatingDock::Deactivate(const float Delay/*= 0.f*/)
{
	if (Delay > 0.f)
	{
		auto Delegate = FTimerDelegate::CreateUObject(this, &AAtomFloatingDock::Deactivate, 0.f);
		GetWorldTimerManager().SetTimer(DeactivationTimerHandle, Delegate, Delay, false);
	}	
	else
	{
		JointSphereComponent->SetHiddenInGame(true);
		SecondLineComponent->SetHiddenInGame(true);

		bIsExtended = false;
		bIsActive = false;
	}
}

void AAtomFloatingDock::Update(const float DeltaTime, const FVector OrientateToward)
{
	if (bIsActive)
	{
		if (!bIsExtended && StepExtension(DeltaTime))
		{
			bIsExtended = true;
			PostExtended();
		}
	}
	else
	{
		if (!bIsRetracted && StepRetraction(DeltaTime))
		{
			bIsRetracted = true;
			PostRetracted();
		}
	}

	if (!bIsExtended)
	{
		// No further updates until extended
		return;
	}

	// Orientate it toward the viewer
	const FVector DirectionToward = (OrientateToward - JointSphereComponent->GetComponentLocation()).GetSafeNormal();
	const FQuat TowardRotation = DirectionToward.ToOrientationQuat();

	// NOTE: The origin of the actor will be the designated target of the text
	const FVector FirstLineLocation = FVector::ZeroVector;
	const FQuat FirstLineRotation = FVector::ForwardVector.ToOrientationQuat();

	// NOTE: The joint sphere draws at the connection point between the lines
	const FVector JointLocation = FirstLineLocation + FirstLineRotation * FVector::ForwardVector * FirstLineLength;
	const FVector JointScale = FVector(LineRadius);
	JointSphereComponent->SetRelativeLocation(JointLocation);

	// NOTE: The second line starts at the joint location
	SecondLineComponent->SetWorldLocation(JointSphereComponent->GetComponentLocation());
	SecondLineComponent->SetWorldRotation((TowardRotation * -FVector::RightVector).ToOrientationQuat());

	UpdateInternal(OrientateToward, TowardRotation);
}

void AAtomFloatingDock::LifeSpanExpired()
{
	if (bIsExtended || bIsActive)
	{
		// Deactivate before destroying
		Deactivate();
		SetLifeSpan(2.f);
	}
	else
	{
		Super::LifeSpanExpired();
	}	
}

bool AAtomFloatingDock::StepExtension(const float DeltaTime)
{
	const float InterpLength = FMath::FInterpConstantTo(FirstLineComponent->RelativeScale3D.X, FirstLineLength, DeltaTime, ExtensionSpeed);

	const FVector FirstLineScale = FVector{ InterpLength, LineRadius, LineRadius };
	FirstLineComponent->SetRelativeScale3D(FirstLineScale);

	return (InterpLength == FirstLineLength);
}

bool AAtomFloatingDock::StepRetraction(const float DeltaTime)
{
	const float InterpLength = FMath::FInterpConstantTo(FirstLineComponent->RelativeScale3D.X, 0.f, DeltaTime, ExtensionSpeed);

	const FVector FirstLineScale = FVector{ InterpLength, LineRadius, LineRadius };
	FirstLineComponent->SetRelativeScale3D(FirstLineScale);

	return (InterpLength == 0.f);
}

void AAtomFloatingDock::PostExtended()
{
	JointSphereComponent->SetHiddenInGame(false);
	SecondLineComponent->SetHiddenInGame(false);
}

void AAtomFloatingDock::PostRetracted()
{
	FirstLineComponent->SetHiddenInGame(true);
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
