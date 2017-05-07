// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomFloatingText.h"
#include "Components/TextRenderComponent.h"


AAtomFloatingText::AAtomFloatingText()
{
	// #AtomTodo: Tweak
	const float TextSize = 1.5f;
	const bool bAllowTextLighting = false;

	{
		static ConstructorHelpers::FObjectFinder<UMaterial> ObjectFinder(TEXT("/Game/UI/Fonts/VRTextMaterial"));
		MaskedTextMaterial = ObjectFinder.Object;
		check(MaskedTextMaterial != nullptr);
	}

	{
		static ConstructorHelpers::FObjectFinder<UMaterialInstance> ObjectFinder(TEXT("/Game/UI/Fonts/TranslucentVRTextMaterial"));
		TranslucentTextMaterial = ObjectFinder.Object;
		check(TranslucentTextMaterial != nullptr);
	}

	UFont* TextFont = nullptr;
	{
		static ConstructorHelpers::FObjectFinder<UFont> ObjectFinder(TEXT("/Game/UI/Fonts/VRText_RobotoLarge"));
		TextFont = ObjectFinder.Object;
		check(TextFont != nullptr);
	}

	{
		TextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Text"));
		check(TextComponent != nullptr);

		TextComponent->SetMobility(EComponentMobility::Movable);
		TextComponent->SetupAttachment(GetRootComponent());

		TextComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

		TextComponent->SetHiddenInGame(true);
		TextComponent->bGenerateOverlapEvents = false;
		TextComponent->SetCanEverAffectNavigation(false);
		TextComponent->bCastDynamicShadow = bAllowTextLighting;
		TextComponent->bCastStaticShadow = false;
		TextComponent->bAffectDistanceFieldLighting = bAllowTextLighting;
		TextComponent->bAffectDynamicIndirectLighting = bAllowTextLighting;


		TextComponent->SetWorldSize(TextSize);

		// Use a custom font.  The text will be visible up close.	   
		TextComponent->SetFont(TextFont);

		// Assign our custom text rendering material.
		TextComponent->SetTextMaterial(MaskedTextMaterial);

		TextComponent->SetTextRenderColor(FLinearColor::White.ToFColor(false));

		// Left justify the text
		TextComponent->SetHorizontalAlignment(EHTA_Left);

	}
}


void AAtomFloatingText::SetText(const FText& NewText)
{
	check(TextComponent != nullptr);
	TextComponent->SetText(NewText);
}


void AAtomFloatingText::SetOpacity(const float NewOpacity)
{
	Super::SetOpacity(NewOpacity);

	const FLinearColor NewColor = FLinearColor(0.6f, 0.6f, 0.6f).CopyWithNewOpacity(NewOpacity);	// #AtomTodo Tweak brightness
	const FColor NewFColor = NewColor.ToFColor(false);

	check(TextComponent != nullptr);
	// 	if( NewOpacity >= 1.0f - KINDA_SMALL_NUMBER )	// #AtomTodo get fading/translucency working again!
	// 	{
	if (TextComponent->GetMaterial(0) != MaskedTextMaterial)
	{
		TextComponent->SetTextMaterial(MaskedTextMaterial);
	}
	// 	}
	// 	else
	// 	{
	// 		if( TextComponent->GetMaterial( 0 ) != TranslucentTextMaterial )
	// 		{
	// 			TextComponent->SetTextMaterial( TranslucentTextMaterial );
	// 		}
	// 	}

	if (NewFColor != TextComponent->TextRenderColor)
	{
		TextComponent->SetTextRenderColor(NewFColor);
	}
}


void AAtomFloatingText::Deactivate(const float Delay)
{
	if (Delay <= 0)
	{
		TextComponent->SetHiddenInGame(true);
	}	

	Super::Deactivate();
}

void AAtomFloatingText::UpdateInternal(const FVector& OrientateToward, const FQuat& TowardRotation)
{
	Super::UpdateInternal(OrientateToward, TowardRotation);

	SetSecondLineLength(TextComponent->GetTextLocalSize().Y);

	TextComponent->SetWorldLocation(GetJointSphereComponent()->GetComponentLocation());
	TextComponent->SetWorldRotation((TowardRotation * FVector::ForwardVector).ToOrientationQuat());
}

void AAtomFloatingText::PostExtended()
{
	Super::PostExtended();

	TextComponent->SetHiddenInGame(false);
}
