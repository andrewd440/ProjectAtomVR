// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "AtomEquippable.h"
#include "FirearmMagazine.generated.h"

UCLASS()
class PROJECTATOMVR_API AFirearmMagazine : public AAtomEquippable
{
	GENERATED_BODY()
	
public:	
	AFirearmMagazine(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	int32 GetCapacity() const;

	USphereComponent* GetLoadTrigger() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Magazine)
	USphereComponent* LoadTrigger;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Magazine)
	int32 Capacity = 30;
};

FORCEINLINE int32 AFirearmMagazine::GetCapacity() const { return Capacity; }
FORCEINLINE USphereComponent* AFirearmMagazine::GetLoadTrigger() const { return LoadTrigger; }