// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "LevelUIComponent.h"
#include "AtomUISystem.h"
#include "AtomUIActor.h"

ULevelUIComponent::ULevelUIComponent()
{

}

void ULevelUIComponent::SpawnUIActor(AAtomUISystem* OwningSystem)
{
	if (UIActor != nullptr)
	{
		UIActor->Destroy();
		UIActor = nullptr;
	}

	if (UIActorClass != nullptr)
	{
		UE_LOG(LogUISystem, Log, TEXT("Level UI Actor Created: %s"), *UIActorClass->GetName());
		UIActor = GetWorld()->SpawnActorDeferred<AAtomUIActor>(UIActorClass, FTransform::Identity, GetOwner(), nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		UIActor->SetFlags(RF_Transient);		
		UIActor->SetUISystem(OwningSystem);

		UIActor->FinishSpawning(FTransform::Identity, true);
		UIActor->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
	}
}

void ULevelUIComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(ULevelUIComponent, UIActorClass))
	{
		SpawnUIActor(nullptr);
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void ULevelUIComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (UIActor != nullptr)
	{
		UIActor->Destroy();
		UIActor = nullptr;
	}

	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

void ULevelUIComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	if (GetWorld()->WorldType == EWorldType::Editor)
	{
		SpawnUIActor(nullptr);
	}
}

void ULevelUIComponent::OnRegister()
{
	Super::OnRegister();

	if (GetWorld()->WorldType == EWorldType::Editor)
	{
		SpawnUIActor(nullptr);
	}
}
