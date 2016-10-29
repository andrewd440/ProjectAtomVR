// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_Firearm.generated.h"

UENUM()
enum class EFirearmNotify : uint8
{
	ShellEject,
};

/**
 * 
 */
UCLASS(const, hidecategories = Object, collapsecategories, meta = (DisplayName = "Play Firearm Notify"))
class PROJECTATOMVR_API UAnimNotify_Firearm : public UAnimNotify
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere)
	EFirearmNotify Type;

private:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
