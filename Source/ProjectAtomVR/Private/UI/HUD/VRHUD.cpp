// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "VRHUD.h"

#include "AtomEquippable.h"
#include "AtomLoadout.h"
#include "AtomLoadoutTemplate.h"
#include "EquippableHUDActor.h"
#include "IConsoleManager.h"
#include "Engine/World.h"
#include "AtomFloatingText.h"
#include "AtomCharacter.h"
#include "HMDCameraComponent.h"
#include "AtomFloatingUI.h"
#include "SBorder.h"
#include "UMGStyle.h"
#include "Messages/AtomLocalMessage.h"
#include "UserWidget.h"
#include "AtomHUDLocalMessageWidget.h"

DEFINE_LOG_CATEGORY(LogVRHUD);

#define LOCTEXT_NAMESPACE "VRHUD"

namespace
{
	static TAutoConsoleVariable<int32> ShowObjectHelpIndicators(TEXT("HUD.ShowObjectHelpIndicators"), 1, TEXT("Toggles if object help indicators are shown."));

	namespace GameStatusTransform
	{
		constexpr float Scale = 40;
		static const FVector2D Res{ 1024, 300 };
		static const FVector Location{ 60, 0, 50 };
		static const FRotator Rotation{ 30, 180, 0 };
	}

	namespace MessageTransform
	{
		constexpr float Scale = 30;
		static const FVector2D Res{ 1024, 1024 };
	}	
}

AVRHUD::AVRHUD()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	bCanBeDamaged = false;

	bShowHelp = true;
}

AAtomPlayerController* AVRHUD::GetPlayerController() const
{
	return PlayerController;
}

void AVRHUD::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);

	PlayerController = Cast<AAtomPlayerController>(NewOwner);
	ensure(IsActorBeingDestroyed() || PlayerController != nullptr);
}

void AVRHUD::Destroyed()
{
	DestroyLoadoutActors(GetCharacter());

	// Destroy all indicators and clear pending ones
	for (auto& ActiveIndicator : ActiveHelpIndicators)
	{
		if (ActiveIndicator.Indicator.IsValid())
		{
			ActiveIndicator.Indicator->Destroy();
			ActiveIndicator.Indicator = nullptr;
		}
	}

	FTimerManager& TimerManager = GetWorldTimerManager();

	for (auto& PendingIndicator : PendingHelpIndicators)
	{
		TimerManager.ClearTimer(PendingIndicator.TimerHandle);
	}

	if (TimerHandle_DefaultTimer.IsValid())
	{
		TimerManager.ClearTimer(TimerHandle_DefaultTimer);
	}

	Super::Destroyed();
}

void AVRHUD::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Update active help with head position
	APawn* Pawn = PlayerController->GetPawn();
	if (Pawn && GEngine->HMDDevice.IsValid() && ActiveHelpIndicators.Num() > 0)
	{
		FVector RoomSpaceHeadLocation;
		FQuat RoomSpaceHeadOrientation;
		GEngine->HMDDevice->GetCurrentOrientationAndPosition( /* Out */ RoomSpaceHeadOrientation, /* Out */ RoomSpaceHeadLocation);
	
		FTransform HeadTransform = FTransform(
			RoomSpaceHeadOrientation,
			RoomSpaceHeadLocation,
			FVector(1.0f));

		HeadTransform = HeadTransform * Pawn->GetTransform(); // Get world transform
		const FVector HeadLocation = HeadTransform.GetLocation();

		for (int32 i = 0; i < ActiveHelpIndicators.Num();)
		{
			if (ActiveHelpIndicators[i].Indicator.IsValid())
			{
				ActiveHelpIndicators[i].Indicator->Update(HeadLocation);
				++i;
			}
			else
			{
				// Remove any expired indicators
				ActiveHelpIndicators.RemoveAt(i);
			}
		}	
	}
}

void AVRHUD::BeginPlay()
{
	Super::BeginPlay();

	// Create UI for match state info
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.ObjectFlags |= RF_Transient;

	GameStatusUI = GetWorld()->SpawnActor<AAtomFloatingUI>(SpawnParams);				
	GameStatusUI->SetSlateWidget(CreateGameStatusWidget(), GameStatusTransform::Res, GameStatusTransform::Scale);

	// Attach game status ui
	if (GetCharacter())
	{
		GameStatusUI->AttachToComponent(GetCharacter()->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		GameStatusUI->SetActorRelativeTransform(FTransform{ GameStatusTransform::Rotation, GameStatusTransform::Location });
		GameStatusUI->ShowUI(true);
	}
	else
	{
		GameStatusUI->ShowUI(false);
	}
}

void AVRHUD::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	GetWorldTimerManager().SetTimer(TimerHandle_DefaultTimer, this, &AVRHUD::DefaultTimer, GetWorldSettings()->GetEffectiveTimeDilation(), true);
}

AAtomCharacter* AVRHUD::GetCharacter() const
{
	return PlayerController ? PlayerController->GetCharacter() : nullptr;
}

void AVRHUD::OnCharacterChanged(AAtomCharacter* OldCharacter)
{
	if (OldCharacter != GetCharacter())
	{
		DestroyLoadoutActors(OldCharacter);

		if (GetCharacter())
		{
			SpawnLoadoutActors();
		}			
	}

	// Attach game status ui
	if (GetCharacter() && GameStatusUI)
	{
		GameStatusUI->AttachToComponent(GetCharacter()->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		GameStatusUI->SetActorRelativeTransform(FTransform{ GameStatusTransform::Rotation, GameStatusTransform::Location });
		GameStatusUI->ShowUI(true);
	}
}

void AVRHUD::ShowHelpIndicator(FHelpIndicatorHandle& HelpHandle, const FText& Text, USceneComponent* AttachParent, 
	const FName AttachSocket, const float Lifetime, const float Delay)
{
	HelpHandle.Handle = NextHelpIndicatorHandle++;

	if (Delay > 0)
	{
		const int32 Index = PendingHelpIndicators.Emplace(HelpHandle, Text, AttachParent, AttachSocket, Lifetime);
	 	FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &AVRHUD::CreatePendingHelpIndicator, HelpHandle.Handle);
		GetWorldTimerManager().SetTimer(PendingHelpIndicators[Index].TimerHandle, TimerDelegate, Delay, false);
	}
	else
	{
		CreateActiveHelpIndicator(HelpHandle, Text, AttachParent, AttachSocket, Lifetime);
	}
}

void AVRHUD::DefaultTimer()
{
	// Update game state text
	auto GameStatusTextPin = GameStatusTextBlock.Pin();
	if (GameStatusTextPin.IsValid())
	{
		if (auto GameState = GetWorld()->GetGameState<AAtomGameState>())
		{
			GameStatusTextPin->SetText(GameState->GetGameStatusText());
		}		
	}
}

void AVRHUD::CreatePendingHelpIndicator(const uint64 Handle)
{
	const int32 Index = PendingHelpIndicators.IndexOfByPredicate([Handle](auto& Indicator) 
	{ 
		return Indicator.HelpHandle.Handle == Handle; 
	});

	check(Index != INDEX_NONE);

	const FPendingHelpIndicator& Indicator = PendingHelpIndicators[Index];
	CreateActiveHelpIndicator(Indicator.HelpHandle, Indicator.Text, Indicator.AttachParent, Indicator.AttachSocket, Indicator.LifeTime);
	PendingHelpIndicators.RemoveAt(Index);
}

void AVRHUD::CreateActiveHelpIndicator(const FHelpIndicatorHandle& Handle, const FText& Text, USceneComponent* AttachParent, const FName AttachSocket, const float Lifetime)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.ObjectFlags |= RF_Transient;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AAtomFloatingText* HelpIndicator = GetWorld()->SpawnActor<AAtomFloatingText>(SpawnParams);
	HelpIndicator->AttachToComponent(AttachParent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachSocket);
	HelpIndicator->SetText(Text);

	HelpIndicator->SetLifeSpan(Lifetime);

	// Make sure this handle does not already exist
	check(ActiveHelpIndicators.FindByPredicate([Handle](const FActiveHelpIndicator& ActiveIndicator) { return ActiveIndicator.HelpHandle == Handle; }) == nullptr);
	ActiveHelpIndicators.Emplace(HelpIndicator, Handle);
}

void AVRHUD::ClearHelpIndicator(FHelpIndicatorHandle& Handle)
{
	if (Handle.IsValid())
	{
		// First check pending indicators
		const int32 PendingIndex = PendingHelpIndicators.IndexOfByPredicate([Handle](const FPendingHelpIndicator& PendingIndicator)
		{
			return PendingIndicator.HelpHandle == Handle;
		});

		if (PendingIndex != INDEX_NONE)
		{
			FPendingHelpIndicator& PendingIndicator = PendingHelpIndicators[PendingIndex];

			if (PendingIndicator.TimerHandle.IsValid())
			{
				GetWorldTimerManager().ClearTimer(PendingIndicator.TimerHandle);
			}		

			PendingHelpIndicators.RemoveAt(PendingIndex);
		}
		else
		{
			// Not in pending indicators, check active indicators
			const int32 ActiveIndex = ActiveHelpIndicators.IndexOfByPredicate([Handle](const FActiveHelpIndicator& ActiveIndicator)
			{
				return ActiveIndicator.HelpHandle == Handle;
			});

			if (ActiveIndex != INDEX_NONE)
			{
				FActiveHelpIndicator& ActiveIndicator = ActiveHelpIndicators[ActiveIndex];

				if (ActiveIndicator.Indicator.IsValid())
				{
					ActiveIndicator.Indicator->Destroy();
					ActiveHelpIndicators.RemoveAt(ActiveIndex);
				}
			}
		}

		Handle.Reset();
	}
}

void AVRHUD::ReceiveLocalMessage(TSubclassOf<class UAtomLocalMessage> MessageClass, const int32 MessageIndex, 
	const FText& MessageText, APlayerState* RelatedPlayerState_1, APlayerState* RelatedPlayerState_2, UObject* OptionalObject)
{
	UE_LOG(LogVRHUD, Log, TEXT("%s"), *MessageText.ToString());
	const UAtomLocalMessage* const DefaultMessage = MessageClass->GetDefaultObject<UAtomLocalMessage>();

	if (DefaultMessage->HUDWidget != nullptr)
	{
		auto Widget = CreateWidget<UUserWidget>(PlayerController, DefaultMessage->HUDWidget);

		if (auto LocalMessageWidget = Cast<UAtomHUDLocalMessageWidget>(Widget))
		{
			LocalMessageWidget->InitializeWithMessage(MessageClass, MessageIndex, MessageText, RelatedPlayerState_1, 
				RelatedPlayerState_2, OptionalObject);
		}

		// Make sure we never leave lifespan at 0
		const float Lifespan = DefaultMessage->DisplayTime > 0 ? DefaultMessage->DisplayTime : 1.0f;

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.ObjectFlags |= RF_Transient;

		auto MessageFloatingUI = GetWorld()->SpawnActor<AAtomFloatingUI>(SpawnParams);
		MessageFloatingUI->SetUMGWidget(Widget, MessageTransform::Res, MessageTransform::Scale);
		MessageFloatingUI->SetLifeSpan(Lifespan);

		// Set position relative to view location
		FVector CameraLoc; FRotator CameraRot;
		PlayerController->GetPlayerViewPoint(CameraLoc, CameraRot);

		const FRotationMatrix CameraRotMat{ CameraRot };
		
		FVector UILoc = CameraLoc;
		UILoc += DefaultMessage->HUDWidgetLocationOffset.X * CameraRotMat.GetScaledAxis(EAxis::X).GetSafeNormal2D();
		UILoc += DefaultMessage->HUDWidgetLocationOffset.Y * CameraRotMat.GetScaledAxis(EAxis::Y).GetSafeNormal2D() + 
			DefaultMessage->HUDWidgetLocationOffset.Z * FVector::UpVector;

		MessageFloatingUI->SetActorLocationAndRotation(UILoc, (CameraLoc - UILoc).ToOrientationQuat());
	}
}

void AVRHUD::SpawnLoadoutActors()
{
	UE_LOG(LogVRHUD, Verbose, TEXT("%s::SpawnLoadoutActors()"), *GetClass()->GetName());

	check(LoadoutActors.Num() == 0);

	UAtomLoadout* Loadout = GetCharacter()->GetLoadout();
	const auto& TemplateSlots = Loadout->GetTemplateSlots();
	auto& LoadoutSlots = Loadout->GetLoadoutSlots();

	LoadoutActors.SetNum(LoadoutSlots.Num());

	for (int32 i = 0; i < TemplateSlots.Num(); ++i)
	{
		FAtomLoadoutSlot& LoadoutSlot = LoadoutSlots[i];
		const FAtomLoadoutTemplateSlot& TemplateSlot = TemplateSlots[i];
		const auto EquippableUIClass = TemplateSlot.ItemClass->GetDefaultObject<AAtomEquippable>()->GetHUDActor();

		// Create the UI only if the item is created. If not, (i.e. not replicated yet) wait until the item changed
		// event tells us it is there.
		if (LoadoutSlot.Item && EquippableUIClass)
		{
			auto LoadoutActor = GetWorld()->SpawnActorDeferred<AEquippableHUDActor>(EquippableUIClass, FTransform::Identity, this);
			LoadoutActor->SetFlags(RF_Transient);
			LoadoutActor->SetEquippable(LoadoutSlot.Item);

			LoadoutActor->FinishSpawning(FTransform::Identity, true);

			LoadoutActors[i] = LoadoutActor;
		}

		LoadoutSlot.OnSlotChanged.AddUObject(this, &AVRHUD::OnLoadoutSlotChanged, i);
	}
}

void AVRHUD::DestroyLoadoutActors(AAtomCharacter* OldCharacter)
{
	UE_LOG(LogVRHUD, Verbose, TEXT("%s::DestroyLoadoutActors()"), *GetClass()->GetName());

	for (auto* LoadoutActor : LoadoutActors)
	{
		if (LoadoutActor)
		{
			LoadoutActor->Destroy();
		}		
	}

	LoadoutActors.Empty();

	// Unbind any slot change events
	if (OldCharacter)
	{
		TArray<FAtomLoadoutSlot>& LoadoutSlots = OldCharacter->GetLoadout()->GetLoadoutSlots();

		for (FAtomLoadoutSlot& Slot : LoadoutSlots)
		{
			Slot.OnSlotChanged.RemoveAll(this);
		}
	}
}

TSharedRef<SWidget> AVRHUD::CreateGameStatusWidget()
{
	TSharedRef<STextBlock> TextBlock = SNew(STextBlock)
		.Text(LOCTEXT("GameStatus", "Game Status"))
		.AutoWrapText(true)
		.Font(FSlateFontInfo{ FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 96 })
		.Justification(ETextJustify::Center);

	GameStatusTextBlock = TextBlock;

	return TextBlock;
}

void AVRHUD::OnLoadoutSlotChanged(ELoadoutSlotChangeType Change, int32 LoadoutIndex)
{
	UAtomLoadout* Loadout = GetCharacter()->GetLoadout();
	const auto& LoadoutSlots = Loadout->GetLoadoutSlots();

	AEquippableHUDActor*& HUDActor = LoadoutActors[LoadoutIndex];

	if ((Change & ELoadoutSlotChangeType::Item) == ELoadoutSlotChangeType::Item)
	{
		AAtomEquippable* NewItem = LoadoutSlots[LoadoutIndex].Item;

		if (NewItem != nullptr)
		{
			if (HUDActor == nullptr)
			{
				// Not created yet, make it now

				const auto& TemplateSlots = Loadout->GetTemplateSlots();
				const auto EquippableUIClass = NewItem->GetHUDActor();

				if (EquippableUIClass)
				{
					HUDActor = GetWorld()->SpawnActorDeferred<AEquippableHUDActor>(EquippableUIClass, FTransform::Identity, this);
					HUDActor->SetFlags(RF_Transient);
					HUDActor->SetEquippable(NewItem);

					HUDActor->FinishSpawning(FTransform::Identity, true);
				}				
			}
			else
			{
				// Just update the owner
				HUDActor->SetEquippable(NewItem);
			}
		}
		else if (HUDActor != nullptr)
		{
			// No equippable owner, so destroy
			HUDActor->Destroy();
			HUDActor = nullptr;
		}
	}

	Change &= ~ELoadoutSlotChangeType::Item; // Item flag is processed, remove it and move on.

	if (Change != ELoadoutSlotChangeType::None && HUDActor != nullptr)
	{
		HUDActor->OnLoadoutChanged(Change, LoadoutSlots[LoadoutIndex]);
	}
}

#undef LOCTEXT_NAMESPACE
