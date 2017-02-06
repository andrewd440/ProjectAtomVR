// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomFloatingUI.h"
#include "WidgetComponent.h"
#include "UserWidget.h"
#include "SWidget.h"


// Sets default values
AAtomFloatingUI::AAtomFloatingUI()
{
	bReplicates = false;
	bNetLoadOnClient = true;

	SetActorEnableCollision(false);

	USceneComponent* SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = SceneComponent;

	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
	WidgetComponent->SetupAttachment(RootComponent);
	WidgetComponent->SetCastShadow(false);
	WidgetComponent->SetTwoSided(true);
}

void AAtomFloatingUI::SetUMGWidget(TSubclassOf<UUserWidget> WidgetClass, const FVector2D InResolution, const float InScale)
{
	check(!SlateWidget.IsValid() && "Multiple widget sets are not supported.");

	UMGWidgetClass = WidgetClass;

	Resolution = InResolution;
	Scale = InScale;

	UpdateWidgetComponent();
}

void AAtomFloatingUI::SetSlateWidget(const TSharedRef<SWidget>& Widget, const FVector2D InResolution, const float InScale)
{
	check(UMGWidgetClass == nullptr && "Multiple widget sets are not supported.");

	SlateWidget = Widget;

	Resolution = InResolution;
	Scale = InScale;

	UpdateWidgetComponent();
}

void AAtomFloatingUI::ShowUI(const bool bShow)
{
	WidgetComponent->SetHiddenInGame(!bShow);
}

void AAtomFloatingUI::UpdateWidgetComponent()
{
	WidgetComponent->SetDrawSize(Resolution);

	if (SlateWidget.IsValid())
	{
		WidgetComponent->SetSlateWidget(SlateWidget);
	}
	else if (UMGWidgetClass != nullptr)
	{
		UMGWidget = CreateWidget<UUserWidget>(GetWorld(), UMGWidgetClass);
		WidgetComponent->SetWidget(UMGWidget);
	}

	// Update scaling. Each pixel of the widget is set to one world unit and then scale is applied.
	const float Aspect = Resolution.X / Resolution.Y;
	WidgetComponent->SetRelativeScale3D(FVector{ 1.f, 1.f / Resolution.X, 1.f / Resolution.Y / Aspect } * Scale);
}

bool AAtomFloatingUI::GetShowUI() const
{
	return WidgetComponent->bHiddenInGame;
}

void AAtomFloatingUI::Destroyed()
{
	if (WidgetComponent != nullptr)
	{
		WidgetComponent->SetSlateWidget(nullptr);
		WidgetComponent->SetWidget(nullptr);
		WidgetComponent = nullptr;
	}

	SlateWidget = nullptr;

	if (UMGWidget != nullptr)
	{
		UMGWidget->MarkPendingKill();
		UMGWidget = nullptr;
	}

	Super::Destroyed();
}
