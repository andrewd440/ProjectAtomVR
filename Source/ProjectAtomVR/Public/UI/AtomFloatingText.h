// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "AtomFloatingDock.h"
#include "AtomFloatingText.generated.h"

/**
 * Draws 3D text in the world along with targeting line cues.
 */
UCLASS()
class PROJECTATOMVR_API AAtomFloatingText : public AAtomFloatingDock
{
	GENERATED_BODY()

public:

	/** Default constructor that sets up CDO properties */
	AAtomFloatingText();

	/** Sets the text to display */
	void SetText(const FText& NewText);

	/** Sets the opacity of the actor */
	virtual void SetOpacity(const float Opacity) override;

protected:

	/** Call this every frame to orientate the text toward the specified transform */
	void UpdateInternal(const FVector& OrientateToward, const FQuat& TowardRotation) override;

private:

	/** The 3D text we're drawing.  Positioned at the end of the second line. */
	UPROPERTY()
	class UTextRenderComponent* TextComponent;

	/** Masked text material.  Used after faded in */
	UPROPERTY()
	class UMaterialInterface* MaskedTextMaterial;

	/** Translucent text material.  Used during fading */
	UPROPERTY()
	class UMaterialInterface* TranslucentTextMaterial;
};
