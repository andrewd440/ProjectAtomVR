// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Components/SceneComponent.h"
#include "LevelActorComponent.generated.h"

UENUM()
enum class ELevelUISpawnType : uint8
{
	WithLevel,
	Manual
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTATOMVR_API ULevelActorComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	ULevelActorComponent();

	TSubclassOf<class AActor> GetActorClass() const { return ActorClass; }

	ELevelUISpawnType GetSpawnType() const { return SpawnType; }

	void SpawnActor();

	/** USceneComponent Interface Begin */
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void OnComponentCreated() override;
	virtual void OnRegister() override;
	/** USceneComponent Interface End */

protected:
	UPROPERTY(EditAnywhere, Category = LevelUIComponent)
	TSubclassOf<class AActor> ActorClass;

	UPROPERTY(EditAnywhere, Category = LevelUIComponent)
	ELevelUISpawnType SpawnType = ELevelUISpawnType::WithLevel;

	UPROPERTY()
	class AActor* Actor = nullptr;
};
