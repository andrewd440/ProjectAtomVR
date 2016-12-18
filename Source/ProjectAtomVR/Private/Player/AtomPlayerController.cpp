// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomPlayerController.h"
#include "AtomCharacter.h"
#include "UI/AtomUISystem.h"
#include "AtomLocalPlayer.h"

DEFINE_LOG_CATEGORY_STATIC(LogAtomPlayerController, Log, All);

AAtomPlayerController::AAtomPlayerController()
{

}

void AAtomPlayerController::BeginPlay()
{
	Super::BeginPlay();	

	if (HasAuthority())
	{
		AAtomGameMode* const GameMode = GetWorld()->GetAuthGameMode<AAtomGameMode>();

		if (UISystem && GameMode)
		{
			UISystem->CreateGameModeUI(GameMode->GetClass());
		}
	}
}

void AAtomPlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	CreateUISystem();
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
		AtomCharacter->SetIsRightHanded(bIsRightHanded);

		if (WidgetInteraction)
		{
			USceneComponent* Attachment = AtomCharacter->GetHandController(bIsRightHanded ? EHand::Right : EHand::Right);
			WidgetInteraction->AttachToComponent(Attachment, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			WidgetInteraction->SetRelativeRotation(FRotator{ -40, 0, 0 });
		}

		if (UISystem)
		{
			UISystem->SpawnCharacterUI();
		}
	}
}

void AAtomPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AAtomPlayerController, RequestedCharacter, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AAtomPlayerController, bIsRightHanded, COND_SkipOwner);
}

void AAtomPlayerController::SetPlayer(UPlayer* InPlayer)
{
	// Set handedness before Super::SetPlayer to setup input correctly
	if (UAtomLocalPlayer* AtomPlayer = Cast<UAtomLocalPlayer>(InPlayer))
	{
		bIsRightHanded = AtomPlayer->GetIsRightHanded();

		Super::SetPlayer(InPlayer);
		ServerSetIsRightHanded(bIsRightHanded);
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
	else if (WidgetInteraction != nullptr)
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

	if (bIsRightHanded)
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
	if (IsLocalController())
	{
		UE_LOG(LogAtomPlayerController, Log, TEXT("Spawned UISystem"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.ObjectFlags |= RF_Transient;
		UISystem = GetWorld()->SpawnActor<AAtomUISystem>(AAtomUISystem::StaticClass(), SpawnParams);
	}
}

void AAtomPlayerController::execRequestCharacterChange(FString Name)
{
	UClass* Class = FindObjectFast<UClass>(nullptr, *Name, false, true);

	if (Class && Class->IsChildOf(AAtomCharacter::StaticClass()))
	{
		ServerRequestCharacterChange(Class);
	}
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

void AAtomPlayerController::ServerSetIsRightHanded_Implementation(bool InbIsRightHanded)
{
	bIsRightHanded = InbIsRightHanded;
}

bool AAtomPlayerController::ServerSetIsRightHanded_Validate(bool InbIsRightHanded)
{
	return true;
}

void AAtomPlayerController::ServerRequestCharacterChange_Implementation(TSubclassOf<AAtomCharacter> CharacterClass)
{
	if (AAtomGameMode* AtomGameMode = GetWorld()->GetAuthGameMode<AAtomGameMode>())
	{
		AtomGameMode->RequestCharacterChange(this, CharacterClass);
	}
}

bool AAtomPlayerController::ServerRequestCharacterChange_Validate(TSubclassOf<AAtomCharacter> CharacterClass)
{
	return true;
}

AAtomCharacter* AAtomPlayerController::GetHero() const
{
	return AtomCharacter;
}

void AAtomPlayerController::SetRequestedCharacter(TSubclassOf<AAtomCharacter> CharacterClass)
{
	RequestedCharacter = CharacterClass;
}

TSubclassOf<AAtomCharacter> AAtomPlayerController::GetRequestedCharacter() const
{
	return RequestedCharacter;
}

bool AAtomPlayerController::IsRightHanded() const
{
	return bIsRightHanded;
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

