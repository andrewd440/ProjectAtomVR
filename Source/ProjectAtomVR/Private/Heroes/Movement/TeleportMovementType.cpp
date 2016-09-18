// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "TeleportMovementType.h"

#include "Components/SplineMeshComponent.h"
#include "Components/SplineComponent.h"

#include "HeroHand.h"


UTeleportMovementType::UTeleportMovementType(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	bIsTeleportActive = false;

	ArcSpline = CreateDefaultSubobject<USplineComponent>(TEXT("ArcSpline"));
}

void UTeleportMovementType::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	const FName TeleportInput = (GetHero()->IsRightHanded()) ? TEXT("TeleportLeft") : TEXT("TeleportRight");
	InputComponent->BindAction(TeleportInput, IE_Pressed, this, &UTeleportMovementType::OnTeleportPressed);
	InputComponent->BindAction(TeleportInput, IE_Released, this, &UTeleportMovementType::OnTeleportReleased);
}

void UTeleportMovementType::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
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
	}

	Super::DestroyComponent(bPromoteChildren);
}

void UTeleportMovementType::OnTeleportPressed()
{
	if (TeleportActor == nullptr && TeleportActorTemplate)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = TEXT("TeleportActor");
		SpawnParams.Owner = GetOwner();
		TeleportActor = GetWorld()->SpawnActor<AActor>(TeleportActorTemplate, SpawnParams);
	}
	else if(TeleportActor)
	{
		TeleportActor->SetActorHiddenInGame(false);
	}

	AHeroHand* const AttachHand = GetHero()->GetNonDominateHand();
	if (AttachHand)
	{
		ArcSpline->AttachToComponent(AttachHand->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		ArcSpline->SetHiddenInGame(false);
	}

	bIsTeleportActive = true;
}

void UTeleportMovementType::OnTeleportReleased()
{
	bIsTeleportActive = false;

	if (TeleportActor)
	{
		TeleportActor->SetActorHiddenInGame(true);
	}

	if (ArcSpline)
	{
		ArcSpline->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		ArcSpline->SetHiddenInGame(true);
	}
}
