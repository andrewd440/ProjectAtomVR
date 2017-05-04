// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomPlayerHUDProxy.h"
#include "Components/WidgetComponent.h"
#include "AtomPlayerState.h"
#include "AtomPlayerNameWidget.h"

namespace PlayerNameTransform
{
	static constexpr float Scale = 40;
	static const FVector2D Res{ 512, 256 };
}

UAtomPlayerHUDProxy::UAtomPlayerHUDProxy()
{

}

void UAtomPlayerHUDProxy::Initialize(class AAtomPlayerState* Player, TSubclassOf<class UAtomPlayerNameWidget> WidgetClass)
{
	PlayerState = Player;

	NameWidgetComponent = NewObject<UWidgetComponent>(GetOuter());	
	NameWidgetComponent->SetCastShadow(false);
	NameWidgetComponent->SetTwoSided(true);
	NameWidgetComponent->bAbsoluteLocation = true;
	NameWidgetComponent->bAbsoluteRotation = true;
	NameWidgetComponent->bAbsoluteScale = true;

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
	if (NameWidgetComponent->IsVisible())
	{
		// Check for attachment update
		AAtomCharacter* Character = PlayerState.IsValid() ? PlayerState->GetAtomCharacter() : nullptr;
		if (Character)
		{
			NameWidgetComponent->SetWorldLocationAndRotation(Character->GetActorLocation() + FVector{ 0,0,200 }, FRotator::ZeroRotator);
		}

		UE_LOG(LogAtom, Log, TEXT("HUD Name position %s"), *NameWidgetComponent->RelativeLocation.ToString());
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