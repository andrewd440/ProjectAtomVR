// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomPlayerHUDProxy.h"
#include "Components/WidgetComponent.h"
#include "AtomPlayerState.h"
#include "AtomPlayerNameWidget.h"
#include "VRHUD.h"

namespace PlayerNameTransform
{
	static constexpr float Scale = 40;
	static const FVector2D Res{ 512, 256 };
}

UAtomPlayerHUDProxy::UAtomPlayerHUDProxy()
{

}

void UAtomPlayerHUDProxy::Initialize(AVRHUD* HUD, AAtomPlayerState* Player, TSubclassOf<class UAtomPlayerNameWidget> WidgetClass)
{
	OwningHUD = HUD;
	PlayerState = Player;

	NameWidgetComponent = NewObject<UWidgetComponent>(GetOuter());	
	NameWidgetComponent->SetCastShadow(false);
	NameWidgetComponent->SetTwoSided(true);
	NameWidgetComponent->bAbsoluteLocation = true;
	NameWidgetComponent->bAbsoluteRotation = true;
	NameWidgetComponent->bAbsoluteScale = true;
	
	// Setup size to represent disired world resolution/scale
	NameWidgetComponent->SetDrawSize(PlayerNameTransform::Res);

	const float Aspect = PlayerNameTransform::Res.X / PlayerNameTransform::Res.Y;
	NameWidgetComponent->SetRelativeScale3D(
		FVector{ 1.f, 1.f / PlayerNameTransform::Res.X, 1.f / PlayerNameTransform::Res.Y / Aspect } * PlayerNameTransform::Scale);

	NameWidgetComponent->RegisterComponent();

	NameWidget = CreateWidget<UAtomPlayerNameWidget>(GetWorld(), WidgetClass);
	NameWidget->SetPlayer(Player);

	NameWidgetComponent->SetWidget(NameWidget);
}

void UAtomPlayerHUDProxy::TickHUD(float DeltaTime)
{
	check(NameWidgetComponent && "UAtomPlayerHUDProxy has not been initialized.");

	APlayerController* PlayerController = GetLocalPlayerController();
	AAtomCharacter* Character = PlayerState.IsValid() ? PlayerState->GetAtomCharacter() : nullptr;
	if (NameWidgetComponent->IsVisible() && PlayerController && Character)
	{
		FVector EyeLocation; FRotator EyeRotation;
		PlayerController->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector NameWidgetTarget = Character->GetPawnViewLocation();
		NameWidgetTarget.Z += 35.f;

		const FQuat LookQuat = (EyeLocation - NameWidgetTarget).ToOrientationQuat();
		NameWidgetComponent->SetWorldLocationAndRotation(NameWidgetTarget, LookQuat);		
	}
}

void UAtomPlayerHUDProxy::SetNameVisible(bool bVisible)
{
	check(NameWidgetComponent && "UAtomPlayerHUDProxy has not been initialized.");
	if (NameWidgetComponent->IsVisible() != bVisible)
	{
		NameWidgetComponent->SetVisibility(bVisible);
		NameWidget->SetPlayerTalking(false);
	}		
}

void UAtomPlayerHUDProxy::SetPlayerTalkingState(bool bIsTalking)
{
	check(NameWidgetComponent && "UAtomPlayerHUDProxy has not been initialized.");
	if (NameWidgetComponent->IsVisible())
	{
		NameWidget->SetPlayerTalking(bIsTalking);
	}
}

void UAtomPlayerHUDProxy::ReceiveLocalMessage(TSubclassOf<class UAtomLocalMessage> MessageClass, const int32 MessageIndex, 
	const FText& MessageText, AAtomPlayerState* RelatedPlayerState_1, AAtomPlayerState* RelatedPlayerState_2, UObject* OptionalObject)
{
	// Nothing at the moment...
}

void UAtomPlayerHUDProxy::NotifyPlayerChangedTeams()
{
	if (NameWidget)
	{
		NameWidget->OnPlayerChangedTeams();
	}
}

class UWorld* UAtomPlayerHUDProxy::GetWorld() const
{
	return GetOuter()->GetWorld();
}

void UAtomPlayerHUDProxy::BeginDestroy()
{
	if (NameWidgetComponent)
	{
		NameWidgetComponent->DestroyComponent();
		NameWidgetComponent = nullptr;
	}

	if (NameWidget)
	{
		NameWidget->ConditionalBeginDestroy();
		NameWidget = nullptr;
	}

	Super::BeginDestroy();
}

APlayerController* UAtomPlayerHUDProxy::GetLocalPlayerController() const
{
	return OwningHUD->GetPlayerController();
}
