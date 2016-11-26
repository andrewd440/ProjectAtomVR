// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "FirearmWidget.h"
#include "FirearmAmmoWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UFirearmAmmoWidget : public UFirearmWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AmmoCount;
};
