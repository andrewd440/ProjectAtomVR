// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomPlayerController.h"
#include "AtomCharacter.h"
#include "AtomLocalPlayer.h"
#include "GameModes/AtomBaseGameMode.h"
#include "AtomGameUserSettings.h"
#include "VRHUD.h"
#include "AtomPlayerState.h"
#include "AtomTeamInfo.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(LogAtomPlayerController, Log, All);

AAtomPlayerController::AAtomPlayerController()
{

}

void AAtomPlayerController::execChangeTeams()
{
	if (auto AtomPlayerState = Cast<AAtomPlayerState>(PlayerState))
	{
		AtomPlayerState->ServerRequestTeamChange(AAtomTeamInfo::INDEX_NO_TEAM);
	}
}

void AAtomPlayerController::SetPawn(APawn* aPawn)
{
	Super::SetPawn(aPawn);

	AAtomCharacter* OldCharacter = AtomCharacter;
	AtomCharacter = Cast<AAtomCharacter>(aPawn);

	const bool bIsNewCharacter = (AtomCharacter != OldCharacter);

	if (AtomCharacter && bIsNewCharacter)
	{
		AtomCharacter->ApplyPlayerSettings(PlayerSettings);
	}

	if (WidgetInteraction)
	{
		if (AtomCharacter)
		{
			if (bIsNewCharacter)
			{
				USceneComponent* Attachment = AtomCharacter->GetHandController(PlayerSettings.bIsRightHanded ?
					EHand::Right : EHand::Left);

				WidgetInteraction->AttachToComponent(Attachment, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				WidgetInteraction->SetRelativeRotation(FRotator{ -40, 0, 0 });
			}
		}
		else
		{
			WidgetInteraction->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		}

		if (WidgetInteraction->IsActive())
		{
			WidgetInteraction->Deactivate();
		}
	}
	
	if (VRHUD && bIsNewCharacter)
	{
		VRHUD->OnCharacterChanged(OldCharacter);
	}
}

void AAtomPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AAtomPlayerController, RequestedCharacter, COND_OwnerOnly);
}

void AAtomPlayerController::SetPlayer(UPlayer* InPlayer)
{
	UAtomLocalPlayer* AtomLocalPlayer = Cast<UAtomLocalPlayer>(InPlayer);

	if (AtomLocalPlayer)
	{
		check(Cast<UAtomGameUserSettings>(GEngine->GameUserSettings));
		UAtomGameUserSettings* GameUserSettings = static_cast<UAtomGameUserSettings*>(GEngine->GameUserSettings);

		// Set player settings before Super::SetPlayer to setup input correctly
		PlayerSettings = GameUserSettings->GetPlayerSettings();

		Super::SetPlayer(InPlayer);

		// SetPlayer first so RPC works
		ServerSetPlayerSettings(PlayerSettings);
		
		if (AtomCharacter && !HasAuthority())
		{
			// Needed for local only settings
			AtomCharacter->ApplyPlayerSettings(PlayerSettings);
		}

		// Create widget interaction for local players
		WidgetInteraction = NewObject<UWidgetInteractionComponent>(this);
		WidgetInteraction->SetIsReplicated(false);
		WidgetInteraction->RegisterComponent();
		WidgetInteraction->Deactivate();
		WidgetInteraction->bShowDebug = true;
	}
	else
	{
		Super::SetPlayer(InPlayer);

		if (WidgetInteraction)
		{
			// Not local player, destroy
			WidgetInteraction->DestroyComponent();
			WidgetInteraction = nullptr;
		}
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

void AAtomPlayerController::SetIgnorePawnInput(bool bNewPawnInput)
{
	IgnorePawnInput= FMath::Max(IgnorePawnInput + (bNewPawnInput ? +1 : -1), 0);
}

void AAtomPlayerController::ClientSetIgnorePawnInput_Implementation(bool bNewPawnInput)
{
	SetIgnorePawnInput(bNewPawnInput);
}

void AAtomPlayerController::ResetIgnorePawnInput()
{
	IgnorePawnInput = 0;
}

bool AAtomPlayerController::IsPawnInputIgnored() const
{
	return (IgnorePawnInput > 0);
}

void AAtomPlayerController::SetCinematicMode(bool bInCinematicMode, bool bAffectsMovement, bool bAffectsTurning)
{
	Super::SetCinematicMode(bInCinematicMode, bAffectsMovement, bAffectsTurning);

	SetIgnorePawnInput(bInCinematicMode);
}

void AAtomPlayerController::GetSeamlessTravelActorList(bool bToEntry, TArray<class AActor *>& ActorList)
{
	Super::GetSeamlessTravelActorList(bToEntry, ActorList);

	if (VRHUD)
	{
		ActorList.Add(VRHUD);
	}
}

void AAtomPlayerController::ClientTravelInternal_Implementation(const FString& URL, enum ETravelType TravelType, bool bSeamless /*= false*/, FGuid MapPackageGuid /*= FGuid()*/)
{
	// #HACK For seamless travel on clients, IKinema likes accessing the physics system too early, before it is create, for IK. Just
	// disable animation blueprint for all AtomCharacters here since a new one will be created after the level transition anyway.
	if (bSeamless)
	{
		for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
		{
			if (auto WorldAtomCharacter = Cast<AAtomCharacter>(*It))
			{
				WorldAtomCharacter->GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
			}
		}
	}

	Super::ClientTravelInternal_Implementation(URL, TravelType, bSeamless, MapPackageGuid);
}

void AAtomPlayerController::ClientSetVRHUD_Implementation(TSubclassOf<class AVRHUD> NewHUDClass)
{
	if (VRHUD)
	{
		VRHUD->Destroy();
		VRHUD = nullptr;
	}

	if (NewHUDClass)
	{
		UE_LOG(LogAtomPlayerController, Log, TEXT("Spawning VRHUD % for %"), *NewHUDClass->GetName(), *GetName());

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.ObjectFlags |= RF_Transient;
		VRHUD = GetWorld()->SpawnActor<AVRHUD>(NewHUDClass, FTransform::Identity, SpawnParams);
	}
}

void AAtomPlayerController::Destroyed()
{
	if (VRHUD)
	{
		VRHUD->Destroy();
		VRHUD = nullptr;
	}

	Super::Destroyed();
}

void AAtomPlayerController::SpawnDefaultHUD()
{
	if (Cast<ULocalPlayer>(Player) == NULL)
	{
		return;
	}

	UE_LOG(LogAtomPlayerController, Verbose, TEXT("SpawnDefaultHUD"));
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = this;
	SpawnInfo.Instigator = Instigator;
	SpawnInfo.ObjectFlags |= RF_Transient;	// We never want to save HUDs into a map
	VRHUD = GetWorld()->SpawnActor<AVRHUD>(SpawnInfo);
}

