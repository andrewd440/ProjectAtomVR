// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Components/SceneComponent.h"
#include "LevelUIComponent.generated.h"

UENUM()
enum class ELevelUISpawnType : uint8
{
	WithLevel,
	Manual
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTATOMVR_API ULevelUIComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	ULevelUIComponent();

	TSubclassOf<class AAtomUIActor> GetUIActor() const { return UIActorClass; }

	ELevelUISpawnType GetSpawnType() const { return SpawnType; }

	void SpawnUIActor(class AAtomUISystem* OwningSystem);

	/** USceneComponent Interface Begin */
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void OnComponentCreated() override;
	virtual void OnRegister() override;
	/** USceneComponent Interface End */

protected:
	UPROPERTY(EditAnywhere, Category = LevelUIComponent)
	TSubclassOf<class AAtomUIActor> UIActorClass;

	UPROPERTY(EditAnywhere, Category = LevelUIComponent)
	ELevelUISpawnType SpawnType = ELevelUISpawnType::WithLevel;

	UPROPERTY()
	class AAtomUIActor* UIActor = nullptr;
};
