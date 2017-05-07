// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomWidgetDock.h"

AAtomWidgetDock::AAtomWidgetDock()
{
	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
	WidgetComponent->SetupAttachment(GetSecondLineComponent());
	WidgetComponent->SetRelativeRotation(FRotator{ 0.f, 90.f, 0.f });
	WidgetComponent->SetCastShadow(false);
	WidgetComponent->SetTwoSided(true);
	WidgetComponent->SetPivot(FVector2D{ 0.f, 1.f });
	WidgetComponent->bAbsoluteScale = true;
}

void AAtomWidgetDock::SetUMGWidget(class UUserWidget* Widget, const FVector2D Resolution, const float Scale)
{
	WidgetComponent->SetDrawSize(Resolution);

	if (Widget != nullptr)
	{
		WidgetComponent->SetWidget(Widget);
	}

	// Update scaling. Each pixel of the widget is set to one world unit and then scale is applied.
	const float Aspect = Resolution.X / Resolution.Y;
	const float ScaleOverAspect = Scale / Aspect;
	WidgetComponent->SetRelativeScale3D(FVector{ 1.f, Scale / Resolution.X, ScaleOverAspect / Resolution.Y});

	SetSecondLineLength(Scale);
}

void AAtomWidgetDock::RecieveLocalMessage(TSubclassOf<class UAtomLocalMessage> MessageClass, const int32 MessageIndex, 
	const FText& MessageText, class AAtomPlayerState* RelatedPlayerState_1, class AAtomPlayerState* RelatedPlayerState_2, UObject* OptionalObject)
{
	UUserWidget* Widget = WidgetComponent->GetUserWidgetObject();

	if (Widget->GetClass()->ImplementsInterface(UAtomLocalMessageInterface::StaticClass()))
	{
		IAtomLocalMessageInterface::Execute_RecieveLocalMessage(Widget, MessageClass, MessageIndex, MessageText,
			RelatedPlayerState_1, RelatedPlayerState_2, OptionalObject);
	}
}

void AAtomWidgetDock::Deactivate(const float InDelay /*= 0.f*/)
{
	if (InDelay <= 0)
	{
		WidgetComponent->SetHiddenInGame(true);
	}

	Super::Deactivate(InDelay);
}

void AAtomWidgetDock::PostExtended()
{
	Super::PostExtended();

	WidgetComponent->SetHiddenInGame(false);
}

void AAtomWidgetDock::UpdateInternal(const FVector& OrientateToward, const FQuat& TowardRotation)
{
	Super::UpdateInternal(OrientateToward, TowardRotation);

	WidgetComponent->SetWorldRotation(TowardRotation);
}
