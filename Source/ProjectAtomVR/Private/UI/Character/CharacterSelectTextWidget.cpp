// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "CharacterSelectTextWidget.h"
#include "TextBlock.h"


void UCharacterSelectTextWidget::SetText(FText Text)
{
	TextBlock->SetText(Text);
}
