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
#include "AtomPlayerState.h"
#include "AtomPlayerNameWidget.h"
#include "OnlineSubsystemUtils.h"
#include "VoiceInterface.h"
#include "CoreOnline.h"
#include "AtomPlayerHUDProxy.h"
#include "AtomLobbyGameState.h"
#include "Messages/AtomEngineMessage.h"
#include "AtomLocalMessageInterface.h"
#include "AtomWidgetDock.h"

DEFINE_LOG_CATEGORY(LogVRHUD);

#define LOCTEXT_NAMESPACE "VRHUD"

namespace
{
	static TAutoConsoleVariable<int32> ShowObjectHelpIndicators(TEXT("HUD.ShowObjectHelpIndicators"), 1, 
		TEXT("Toggles if object help indicators are shown."));

	namespace GameStatusDockParams
	{
		static constexpr float LineLength = 20.f;
		static constexpr float ExtensionSpeed = 30.f;
		static constexpr float LineRadius = 0.15f;
		static constexpr float Scale = 25.f;
		static constexpr float RotationSpeed = 20.f;
		static const FVector2D Res{ 1024, 256 };
		static const FVector Location{ 10.f, -10.f, -0.30f }; // Z is scaled by player height
		static const FRotator Rotation{ 0.f, -25.f, 0.f };
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
	bShowNames = true;
}

AAtomPlayerController* AVRHUD::GetPlayerController() const
{
	return PlayerController;
}

void AVRHUD::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);

	PlayerController = Cast<AAtomPlayerController>(NewOwner);
}

void AVRHUD::Destroyed()
{
	IOnlineVoicePtr VoiceInt = Online::GetVoiceInterface(GetWorld());
	check(VoiceInt.IsValid());
	VoiceInt->ClearOnPlayerTalkingStateChangedDelegate_Handle(OnPlayerTalkingStateChangedHandle);

	for (auto Proxy : PlayerHUDProxies)
	{
		Proxy->ConditionalBeginDestroy();
	}
	PlayerHUDProxies.Empty();

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

	// Update player HUD proxies
	for (auto Proxy : PlayerHUDProxies)
	{
		Proxy->TickHUD(DeltaSeconds);
	}

	// Update active help with head position
	if (PlayerController != nullptr)
	{
		FVector HeadLocation; FRotator HeadRot;
		PlayerController->GetActorEyesViewPoint(HeadLocation, HeadRot);

		for (int32 i = 0; i < ActiveHelpIndicators.Num();)
		{
			if (ActiveHelpIndicators[i].Indicator.IsValid())
			{
				ActiveHelpIndicators[i].Indicator->Update(DeltaSeconds, HeadLocation);
				++i;
			}
			else
			{
				// Remove any expired indicators
				ActiveHelpIndicators.RemoveAt(i);
			}
		}

		if (GameStatusDock)
		{
			GameStatusDock->Update(DeltaSeconds, HeadLocation);
		}

		if (GameStatusDock)
		{			
			// Update game status dock with location and eased rotation
			const float DesiredYaw = HeadRot.Yaw + GameStatusDockParams::Rotation.Yaw;
			const float CurrentYaw = GameStatusDock->GetActorRotation().Yaw;
			const float InteropYaw = FMath::FInterpTo(CurrentYaw, DesiredYaw, DeltaSeconds, GameStatusDockParams::RotationSpeed);

			const FVector Forward = FRotator{ 0.f, InteropYaw, 0.f }.Vector();	
			const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward);
			GameStatusDock->SetActorLocationAndRotation(HeadLocation + 
				Forward * GameStatusDockParams::Location.X +
				Right * GameStatusDockParams::Location.Y +
				FVector::UpVector * GameStatusDockParams::Location.Z * PlayerController->GetPlayerSettings().PlayerHeight, 
				FRotator{ 0.f, InteropYaw, 0.f });
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

	// Hook into voice change events
	IOnlineVoicePtr VoiceInt = Online::GetVoiceInterface(GetWorld());
	auto OnPlayerTalkingDelegate = FOnPlayerTalkingStateChangedDelegate::CreateUObject(this, &AVRHUD::OnPlayerTalkingStateChanged);
	OnPlayerTalkingStateChangedHandle = VoiceInt->AddOnPlayerTalkingStateChangedDelegate_Handle(OnPlayerTalkingDelegate);

	// Add all player join events if our playerstate is initialized
	if (PlayerController->PlayerState != nullptr)
	{
		AddAllPlayerStateProxies();
	}
}

void AVRHUD::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	GetWorldTimerManager().SetTimer(TimerHandle_DefaultTimer, this, &AVRHUD::DefaultTimer, GetWorldSettings()->GetEffectiveTimeDilation(), true);

	// Create game status dock
	if (GameStatusWidgetClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.ObjectFlags |= RF_Transient;

		GameStatusDock = GetWorld()->SpawnActor<AAtomWidgetDock>(SpawnParams);		
		GameStatusDock->SetFirstLineLength(GameStatusDockParams::LineLength);
		GameStatusDock->SetLineRadius(GameStatusDockParams::LineRadius);
		GameStatusDock->SetExtensionSpeed(GameStatusDockParams::ExtensionSpeed);

		auto Widget = CreateWidget<UUserWidget>(PlayerController, GameStatusWidgetClass);
		GameStatusDock->SetUMGWidget(Widget, GameStatusDockParams::Res, GameStatusDockParams::Scale);
	}
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

}

void AVRHUD::OnPlayerTalkingStateChanged(TSharedRef<const FUniqueNetId> TalkerId, bool bIsTalking)
{
	auto Found = PlayerHUDProxies.FindByPredicate([&TalkerId](auto Proxy) 
	{
		auto Player = Proxy->GetPlayer();
		return Player && Player->UniqueId == TalkerId;
	});

	if (Found)
	{
		(*Found)->SetPlayerTalkingState(bIsTalking);
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
	HelpIndicator->Activate();

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
					ActiveIndicator.Indicator->SetLifeSpan(.1f); // Allow time for deactivation
				}
			}
		}

		Handle.Reset();
	}
}

void AVRHUD::ReceiveLocalMessage(TSubclassOf<class UAtomLocalMessage> MessageClass, const int32 MessageIndex, 
	const FText& MessageText, APlayerState* RelatedPlayerState_1, APlayerState* RelatedPlayerState_2, UObject* OptionalObject)
{
	auto AtomPlayerState_1 = CastChecked<AAtomPlayerState>(RelatedPlayerState_1, ECastCheckedType::NullAllowed);
	auto AtomPlayerState_2 = CastChecked<AAtomPlayerState>(RelatedPlayerState_2, ECastCheckedType::NullAllowed);

	const auto DefaultMessage = MessageClass->GetDefaultObject<UAtomLocalMessage>();

	if (DefaultMessage->IsStatusMessage(MessageIndex))
	{
		GameStatusDock->Activate();
		GameStatusDock->RecieveLocalMessage(MessageClass, MessageIndex, MessageText, AtomPlayerState_1,
			AtomPlayerState_2, OptionalObject);
		GameStatusDock->Deactivate(DefaultMessage->GetStatusMessageDuration(MessageIndex));
	}
	else if (DefaultMessage->HUDWidget != nullptr)
	{
		auto Widget = CreateWidget<UUserWidget>(PlayerController, DefaultMessage->HUDWidget);

		if (Widget->GetClass()->ImplementsInterface(UAtomLocalMessageInterface::StaticClass()))
		{
			IAtomLocalMessageInterface::Execute_RecieveLocalMessage(Widget, MessageClass, MessageIndex, MessageText, 
				AtomPlayerState_1, AtomPlayerState_2, OptionalObject);
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

	if (auto EngineMessage = Cast<const UAtomEngineMessage>(DefaultMessage))
	{
		HandleEngineMessage(EngineMessage, static_cast<EAtomEngineMessageIndex>(MessageIndex), MessageText,
			AtomPlayerState_1, AtomPlayerState_2, OptionalObject);
	}
}

void AVRHUD::HandleEngineMessage(const UAtomEngineMessage* DefaultMessage, const EAtomEngineMessageIndex MessageIndex,
	const FText& MessageText, AAtomPlayerState* RelatedPlayerState_1, AAtomPlayerState* RelatedPlayerState_2, UObject* OptionalObject)
{
	switch (MessageIndex)
	{
	case EAtomEngineMessageIndex::Entered:
		OnPlayerJoinedGame(RelatedPlayerState_1);
		break;
	case EAtomEngineMessageIndex::Left:
		OnPlayerLeftGame(RelatedPlayerState_1);
		break;
	}
}

void AVRHUD::AddAllPlayerStateProxies()
{
	ensureMsgf(PlayerController->PlayerState != nullptr, 
		TEXT("PlayerState should be valid before adding proxies to prevent making a proxy for yourself."));

	TArray<APlayerState*> Players = GetWorld()->GetGameState()->PlayerArray;
	for (auto Player : Players)
	{
		auto AtomPlayer = (Player != PlayerController->PlayerState) ? Cast<AAtomPlayerState>(Player) : nullptr;
		if (AtomPlayer)
		{
			OnPlayerJoinedGame(AtomPlayer);
		}
	}
}

void AVRHUD::HandleTeamChange()
{
	const bool bIsLobby = (GetWorld()->GetGameState<AAtomLobbyGameState>() != nullptr);
	auto MyState = CastChecked<AAtomPlayerState>(PlayerController->PlayerState, ECastCheckedType::NullAllowed);

	for (auto Proxy : PlayerHUDProxies)
	{
		const bool bIsSameTeam = !MyState || MyState->GetTeam() == Proxy->GetPlayer()->GetTeam();
		Proxy->SetNameVisible(bShowNames && (bIsLobby || bIsSameTeam));
	}
}

void AVRHUD::OnPlayerJoinedGame(AAtomPlayerState* Player)
{
	if (!HasActorBegunPlay() || PlayerController->PlayerState == nullptr)
		return; // Wait for player state replication and begin play

	check(PlayerHUDProxies.FindByPredicate([Player](UAtomPlayerHUDProxy* Proxy)
	{
		return Proxy->GetPlayer() == Player;
	}) == nullptr && "New players should not have a proxy already");

	UE_LOG(LogVRHUD, Log, TEXT("Creating HUD proxy for %d"), Player->PlayerId);

	auto Proxy = NewObject<UAtomPlayerHUDProxy>(this);
	Proxy->Initialize(this, Player, PlayerNameWidgetClass);

	// Show name in lobby and for same team
	bool bNameVisible = (GetWorld()->GetGameState<AAtomLobbyGameState>() != nullptr);

	if (!bNameVisible)
	{
		auto MyState = CastChecked<AAtomPlayerState>(PlayerController->PlayerState, ECastCheckedType::NullAllowed);
		bNameVisible = (!MyState || MyState->GetTeam() == Player->GetTeam());
	}

	Proxy->SetNameVisible(bShowNames && bNameVisible);
	PlayerHUDProxies.Push(Proxy);
}

void AVRHUD::OnPlayerLeftGame(AAtomPlayerState* Player)
{
	UE_LOG(LogVRHUD, Log, TEXT("Destroying HUD proxy for %d"), Player->PlayerId);

	int32 Index = PlayerHUDProxies.IndexOfByPredicate([Player](UAtomPlayerHUDProxy* Proxy)
	{
		return Proxy->GetPlayer() == Player;
	});

	if (Index != INDEX_NONE)
	{
		UAtomPlayerHUDProxy* Proxy = PlayerHUDProxies[Index];
		PlayerHUDProxies.RemoveAt(Index);

		Proxy->ConditionalBeginDestroy();
	}	

	check(PlayerHUDProxies.IndexOfByPredicate([Player](UAtomPlayerHUDProxy* Proxy)
	{
		return Proxy->GetPlayer() == Player;
	}) == INDEX_NONE && "No players should match by pointer at this point");
}

void AVRHUD::NotifyPlayerChangedTeams(AAtomPlayerState* Player)
{
	if (Player == PlayerController->PlayerState)
	{
		HandleTeamChange();
		return;
	}

	auto Found = PlayerHUDProxies.FindByPredicate([Player](auto Proxy)
	{
		return Proxy->GetPlayer() == Player;
	});
	
	if (Found)
	{
		UAtomPlayerHUDProxy* Proxy = *Found;
		Proxy->NotifyPlayerChangedTeams();

		// Show name in lobby and for same team
		bool bNameVisible = (GetWorld()->GetGameState<AAtomLobbyGameState>() != nullptr);

		if (!bNameVisible)
		{
			auto MyState = Cast<AAtomPlayerState>(PlayerController->PlayerState);
			bNameVisible = (!MyState || MyState->GetTeam() == Player->GetTeam());
		}

		Proxy->SetNameVisible(bShowNames && bNameVisible);
	}
}

void AVRHUD::OnPlayerStateInitialized()
{
	if (HasActorBegunPlay())
	{
		AddAllPlayerStateProxies();
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
