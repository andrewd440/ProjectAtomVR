// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "LevelActorComponent.h"
#include "AtomHUDActor.h"

ULevelActorComponent::ULevelActorComponent()
{

}

void ULevelActorComponent::SpawnActor()
{
	if (Actor != nullptr)
	{
		Actor->Destroy();
		Actor = nullptr;
	}

	if (ActorClass != nullptr)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.ObjectFlags |= RF_Transient;
		Actor = GetWorld()->SpawnActor<AActor>(ActorClass, FTransform::Identity, SpawnParams);
		Actor->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
	}
}

#if WITH_EDITOR
void ULevelActorComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(ULevelActorComponent, ActorClass))
	{
		SpawnActor();
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void ULevelActorComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	if (GetWorld()->WorldType == EWorldType::Editor)
	{
		SpawnActor();
	}
}

void ULevelActorComponent::OnRegister()
{
	Super::OnRegister();

	if (GetWorld()->WorldType == EWorldType::Editor)
	{
		SpawnActor();
	}
}
