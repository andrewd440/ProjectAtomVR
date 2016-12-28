// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CharacterSelectTextWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UCharacterSelectTextWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetText(FText Text);

protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TextBlock;
};
