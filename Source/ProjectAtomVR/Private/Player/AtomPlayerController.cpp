// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomPlayerController.h"
#include "AtomCharacter.h"
#include "UI/AtomUISystem.h"
#include "AtomLocalPlayer.h"
#include "GameModes/AtomBaseGameMode.h"
#include "AtomGameUserSettings.h"

DEFINE_LOG_CATEGORY_STATIC(LogAtomPlayerController, Log, All);

AAtomPlayerController::AAtomPlayerController()
{

}

void AAtomPlayerController::BeginPlay()
{
	Super::BeginPlay();	

	if (HasAuthority())
	{
		// ReceivedGameModeClass not called for Authority, so create gamemode ui here.
		AAtomBaseGameMode* const GameMode = GetWorld()->GetAuthGameMode<AAtomBaseGameMode>();

		if (UISystem && GameMode)
		{
			UISystem->CreateGameModeUI(GameMode->GetClass());
		}
	}

	if (UISystem != nullptr)
	{
		UISystem->CreateLevelUI();
	}
}

void AAtomPlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (IsLocalController())
	{
		CreateUISystem();
	}		
}

void AAtomPlayerController::SetPawn(APawn* aPawn)
{
	const bool IsNewPawn = (AtomCharacter != aPawn);
	if (IsNewPawn && AtomCharacter)
	{
		if (WidgetInteraction)
		{
			WidgetInteraction->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		}

		if (UISystem)
		{
			UISystem->DestroyCharacterUI();
		}		
	}

	Super::SetPawn(aPawn);

	AtomCharacter = Cast<AAtomCharacter>(aPawn);

	if (IsNewPawn && AtomCharacter)
	{
		AtomCharacter->ApplyPlayerSettings(PlayerSettings);

		if (WidgetInteraction)
		{
			USceneComponent* Attachment = AtomCharacter->GetHandController(PlayerSettings.bIsRightHanded ? EHand::Right : EHand::Left);
			WidgetInteraction->AttachToComponent(Attachment, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			WidgetInteraction->SetRelativeRotation(FRotator{ -40, 0, 0 });
		}

		if (UISystem)
		{
			UISystem->CreateCharacterUI();
		}
	}
}

void AAtomPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AAtomPlayerController, RequestedCharacter, COND_OwnerOnly);
}

void AAtomPlayerController::SetPlayer(UPlayer* InPlayer)
{
	// Set player settings before Super::SetPlayer to setup input correctly
	if (UAtomLocalPlayer* AtomPlayer = Cast<UAtomLocalPlayer>(InPlayer))
	{
		check(Cast<UAtomGameUserSettings>(GEngine->GameUserSettings));
		UAtomGameUserSettings* GameUserSettings = static_cast<UAtomGameUserSettings*>(GEngine->GameUserSettings);

		PlayerSettings = GameUserSettings->GetPlayerSettings();

		Super::SetPlayer(InPlayer);

		ServerSetPlayerSettings(PlayerSettings);
		
		if (AtomCharacter && !HasAuthority())
		{
			// Needed for local only settings
			AtomCharacter->ApplyPlayerSettings(PlayerSettings);
		}
	}
	else
	{
		Super::SetPlayer(InPlayer);
	}	

	// Create widget interaction for local controllers
	if (IsLocalController())
	{
		WidgetInteraction = NewObject<UWidgetInteractionComponent>(this);
		WidgetInteraction->SetIsReplicated(false);
		WidgetInteraction->RegisterComponent();
		WidgetInteraction->Deactivate();
		WidgetInteraction->bShowDebug = true;
	}
	else if (WidgetInteraction)
	{
		WidgetInteraction->DestroyComponent();
		WidgetInteraction = nullptr;
	}
}

void AAtomPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	FName MenuAction;
	FName MenuClickAction;

	if (PlayerSettings.bIsRightHanded)
	{
		MenuAction = TEXT("Menu_Right");
		MenuClickAction = TEXT("MenuClick_Right");
	}
	else
	{
		MenuAction = TEXT("Menu_Left");
		MenuClickAction = TEXT("MenuClick_Left");
	}

	InputComponent->BindAction(MenuAction, IE_Pressed, this, &AAtomPlayerController::OnMenuButtonPressed).bConsumeInput = true;
	InputComponent->BindAction(MenuClickAction, IE_Pressed, this, &AAtomPlayerController::OnMenuClickPressed).bConsumeInput = false;
	InputComponent->BindAction(MenuClickAction, IE_Released, this, &AAtomPlayerController::OnMenuClickReleased).bConsumeInput = false;
}

void AAtomPlayerController::CreateUISystem()
{
	check(IsLocalController());

	UE_LOG(LogAtomPlayerController, Log, TEXT("Spawned UISystem"));

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.ObjectFlags |= RF_Transient;
	UISystem = GetWorld()->SpawnActor<AAtomUISystem>(AAtomUISystem::StaticClass(), SpawnParams);
}

void AAtomPlayerController::OnMenuButtonPressed()
{
	if (WidgetInteraction->IsActive())
	{
		WidgetInteraction->Deactivate();
		SetInputMode(FInputModeGameOnly{});
	}
	else
	{
		WidgetInteraction->Activate();
		SetInputMode(FInputModeGameAndUI{});
	}
}

void AAtomPlayerController::OnMenuClickPressed()
{
	if (WidgetInteraction->IsActive())
	{
		WidgetInteraction->PressPointerKey(EKeys::LeftMouseButton);
	}
}

void AAtomPlayerController::OnMenuClickReleased()
{
	if (WidgetInteraction->IsActive())
	{
		WidgetInteraction->ReleasePointerKey(EKeys::LeftMouseButton);
	}
}

void AAtomPlayerController::ServerSetPlayerSettings_Implementation(FAtomPlayerSettings InPlayerSettings)
{
	PlayerSettings = InPlayerSettings;

	if (AtomCharacter)
	{
		AtomCharacter->ApplyPlayerSettings(PlayerSettings);
	}
}

bool AAtomPlayerController::ServerSetPlayerSettings_Validate(FAtomPlayerSettings)
{
	return true;
}

void AAtomPlayerController::ServerRequestCharacterChange_Implementation(TSubclassOf<AAtomCharacter> CharacterClass)
{
	if (AAtomBaseGameMode* AtomGameMode = GetWorld()->GetAuthGameMode<AAtomBaseGameMode>())
	{
		AtomGameMode->RequestCharacterChange(this, CharacterClass);
	}
}

bool AAtomPlayerController::ServerRequestCharacterChange_Validate(TSubclassOf<AAtomCharacter> CharacterClass)
{
	return true;
}

AAtomCharacter* AAtomPlayerController::GetCharacter() const
{
	return AtomCharacter;
}

const FAtomPlayerSettings& AAtomPlayerController::GetPlayerSettings() const
{
	return PlayerSettings;
}

void AAtomPlayerController::SetRequestedCharacter(TSubclassOf<AAtomCharacter> CharacterClass)
{
	RequestedCharacter = CharacterClass;
}

TSubclassOf<AAtomCharacter> AAtomPlayerController::GetRequestedCharacter() const
{
	return RequestedCharacter;
}

void AAtomPlayerController::UnFreeze()
{
	ServerRestartPlayer();
}

void AAtomPlayerController::CreateCharacterUI()
{
	check(UISystem);

	UISystem->CreateCharacterUI();
}

void AAtomPlayerController::NotifyLoadedWorld(FName WorldPackageName, bool bFinalDest)
{
	Super::NotifyLoadedWorld(WorldPackageName, bFinalDest);

	if (!UISystem && IsLocalController())
	{
		CreateUISystem();
	}

	if (UISystem != nullptr)
	{
		UISystem->CreateLevelUI();
	}
}

void AAtomPlayerController::ReceivedGameModeClass(TSubclassOf<class AGameModeBase> GameModeClass)
{
	Super::ReceivedGameModeClass(GameModeClass);

	if (UISystem)
	{
		UISystem->CreateGameModeUI(GameModeClass);
	}
}

void AAtomPlayerController::Destroyed()
{
	if (UISystem)
	{
		UISystem->Destroy();
		UISystem = nullptr;
	}

	Super::Destroyed();
}

