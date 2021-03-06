// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomControlPoint.h"
#include "AtomCharacter.h"
#include "AtomPlayerState.h"
#include "AtomTeamInfo.h"
#include "Components/PrimitiveComponent.h"
#include "AtomObjectiveMessage.h"

AAtomControlPoint::AAtomControlPoint()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	//PrimaryActorTick.TickInterval = 0.05f; // Only used to update TeamControl.
	//NetUpdateFrequency = .05f;

	CaptureBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("CaptureBounds"));
	CaptureBounds->SetBoxExtent(FVector{ 250, 250, 100 }, false);
	CaptureBounds->SetCollisionProfileName(AtomCollisionProfiles::ObjectiveTrigger);
	CaptureBounds->OnComponentBeginOverlap.AddDynamic(this, &AAtomControlPoint::OnBoundsBeginOverlap);
	CaptureBounds->OnComponentEndOverlap.AddDynamic(this, &AAtomControlPoint::OnBoundsEndOverlap);
	CaptureBounds->bGenerateOverlapEvents = false; // Start deactivated until control point is activated

	RootComponent = CaptureBounds;

	OutlineMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OutlineMesh"));
	OutlineMesh->bGenerateOverlapEvents = false;
	OutlineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OutlineMesh->SetupAttachment(CaptureBounds);

	InnerSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InnerSphere"));
	InnerSphere->bGenerateOverlapEvents = false;
	InnerSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InnerSphere->SetupAttachment(CaptureBounds);

	OuterSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OuterSphere"));
	OuterSphere->bGenerateOverlapEvents = false;
	OuterSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OuterSphere->SetupAttachment(InnerSphere);

	ObjectiveMessageClass = UAtomObjectiveMessage::StaticClass();

	TeamOverlaps.SetNum(2); // Most, if not all, games will have 2 teams
	bIsActive = false;
	bIsCaptured = false;

	SetActorHiddenInGame(true);
}

void AAtomControlPoint::Activate(const float Delay/* = 0.f*/)
{
	if (Delay > 0)
	{
		GetWorldTimerManager().SetTimer(ActivationHandle, this, &AAtomControlPoint::OnActivated, Delay);
	}
	else
	{
		OnActivated();
	}
}

void AAtomControlPoint::Deactivate()
{
	if (HasAuthority())
	{
		PrimaryActorTick.SetTickFunctionEnable(false);
	}

	SetActorHiddenInGame(true);
	GetWorldTimerManager().ClearTimer(ActivationHandle);
	TeamOverlaps.Empty(2);
	CaptureBounds->bGenerateOverlapEvents = false;
	bIsActive = false;
}

bool AAtomControlPoint::IsActive()
{
	return bIsActive;
}

AAtomTeamInfo* AAtomControlPoint::GetControllingTeam() const
{
	return ControllingTeam;
}

bool AAtomControlPoint::IsCaptured() const
{
	return bIsCaptured;
}

TArray<AAtomPlayerState*> AAtomControlPoint::GetActiveControllingTeamMembers() const
{
	TArray<AAtomPlayerState*> ActivePlayerStates;

	// Get initial overlaps
	TArray<AActor*> ActiveOverlaps;
	CaptureBounds->GetOverlappingActors(ActiveOverlaps, AAtomCharacter::StaticClass());

	for (AActor* InitialOverlap : ActiveOverlaps)
	{
		auto PlayerState = Cast<AAtomPlayerState>(CastChecked<AAtomCharacter>(InitialOverlap)->PlayerState);

		if (PlayerState && PlayerState->GetTeam() == ControllingTeam)
		{
			ActivePlayerStates.Add(PlayerState);
		}
	}

	return ActivePlayerStates;
}

void AAtomControlPoint::InitializeObjective()
{
	Super::InitializeObjective();	
}

void AAtomControlPoint::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	OutlineMesh->CreateAndSetMaterialInstanceDynamic(0);
}

void AAtomControlPoint::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAtomControlPoint, ControllingTeam);
	DOREPLIFETIME(AAtomControlPoint, Control);
	DOREPLIFETIME(AAtomControlPoint, bIsActive);
	DOREPLIFETIME(AAtomControlPoint, ActivationTimestamp);
	DOREPLIFETIME(AAtomControlPoint, bIsCaptured);
}

void AAtomControlPoint::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	check(HasAuthority() && "Only authority should be ticking!");

	// Modify TeamControl based on ControlState
	if (ControlState == EControlState::Capturing)
	{
		// Get overlapping team count
		const int32 TeamInfluence = FMath::Min(MaxTeamInfluence, TeamOverlaps[ControllingTeam->TeamId].Num());
		const float Rate = ControlRate + (TeamInfluence - 1) * ControlRateMultiplier * ControlRate;
		ensure(Rate > 0);

		Control = FMath::Min(1.f, Control + Rate * DeltaSeconds);
		
		if (Control == 1.f && !bIsCaptured)
		{
			bIsCaptured = true;
			OnCapturedChanged();
			BroadcastTeamMessage(ControllingTeam, UAtomObjectiveMessage::EType::CapturedObjective);
			OnCapturedEvent.Broadcast();
		}
	}
	else if (ControlState == EControlState::Lossing)
	{
		// Get overlapping enemy count
		int32 TeamInfluence = 0;
		for (auto Overlaps : TeamOverlaps)
		{
			TeamInfluence += Overlaps.Num();
		}

		TeamInfluence -= TeamOverlaps[ControllingTeam->TeamId].Num();
		TeamInfluence = FMath::Min(MaxTeamInfluence, TeamInfluence);

		const float Rate = ControlRate + (TeamInfluence - 1) * ControlRateMultiplier * ControlRate;
		ensure(Rate > 0);

		Control = FMath::Max(0.f, Control - Rate * DeltaSeconds);

		if (Control == 0.f && bIsCaptured)
		{
			bIsCaptured = false;
			BroadcastTeamMessage(ControllingTeam, UAtomObjectiveMessage::EType::LostObjective);
			ControllingTeam = nullptr;
			UpdateControlState();
			OnCapturedChanged();			
		}
	}
}

void AAtomControlPoint::Reset()
{
	SetActorHiddenInGame(true);	

	ControllingTeam = nullptr;
	ControlState = EControlState::Contested;
	Control = 0.f;
	bIsCaptured = false;

	OnCapturedChanged();

	Super::Reset();
}

void AAtomControlPoint::OnRep_ActivationTimestamp()
{

}

void AAtomControlPoint::OnRep_ControllingTeam()
{
	if (bIsCaptured && ControllingTeam)
	{
		OnCapturedChanged();
		OnCapturedEvent.Broadcast();
	}
}

void AAtomControlPoint::OnRep_IsActive()
{
	if (bIsActive)
	{
		OnActivated();
	}
	else
	{
		Deactivate();
	}	
}

void AAtomControlPoint::OnRep_IsCaptured()
{		
	if (bIsCaptured && ControllingTeam)
	{
		OnCapturedChanged();
		OnCapturedEvent.Broadcast();
	}
	else if (!bIsCaptured)
	{
		OnCapturedChanged();
	}
}

void AAtomControlPoint::OnBoundsBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ensureMsgf(Cast<AAtomCharacter>(OtherActor) != nullptr, TEXT("Control points should only respond to AtomCharacter overlaps."));

	if (auto Character = Cast<AAtomCharacter>(OtherActor))
	{
		if (auto PlayerState = Cast<AAtomPlayerState>(Character->PlayerState))
		{
			if (AAtomTeamInfo* Team = PlayerState->GetTeam())
			{
				const int32 TeamId = Team->TeamId;

				if (!TeamOverlaps.IsValidIndex(TeamId))
				{
					TeamOverlaps.SetNum(TeamId + 1, false);
				}

				check(TeamOverlaps[TeamId].Find(Character) == INDEX_NONE);
				TeamOverlaps[TeamId].Add(Character);
				UpdateControlState();
			}
		}
	}
}

void AAtomControlPoint::OnBoundsEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ensureMsgf(Cast<AAtomCharacter>(OtherActor) != nullptr, TEXT("Control points should only respond to AtomCharacter overlaps."));

	if (!bIsActive) // TeamOverlaps are cleared on deactivate
		return;

	if (auto Character = Cast<AAtomCharacter>(OtherActor))
	{
		if (auto PlayerState = Cast<AAtomPlayerState>(Character->PlayerState))
		{
			if (AAtomTeamInfo* Team = PlayerState->GetTeam())
			{
				const int32 TeamId = Team->TeamId;

				check(TeamOverlaps.IsValidIndex(TeamId));
				TeamOverlaps[TeamId].RemoveSingle(Character);
				UpdateControlState();
			}
		}
		else
		{
			// Character may be killed and no longer have a playerstate. Make sure the character is removed from any
			// team overlaps.
			for (auto& TeamOverlap : TeamOverlaps)
			{
				TeamOverlap.RemoveSingle(Character);
			}
		}
	}
}

void AAtomControlPoint::OnActivated()
{
	bIsActive = true;
	SetActorHiddenInGame(false);

	if (HasAuthority())
	{
		CaptureBounds->bGenerateOverlapEvents = true; // Activate overlaps
		PrimaryActorTick.SetTickFunctionEnable(true);

		// Get initial overlaps
		TArray<AActor*> InitalOverlaps;
		CaptureBounds->GetOverlappingActors(InitalOverlaps, AAtomCharacter::StaticClass());

		for (AActor* InitialOverlap : InitalOverlaps)
		{
			auto Character = CastChecked<AAtomCharacter>(InitialOverlap);
			auto PlayerState = Cast<AAtomPlayerState>(Character->PlayerState);

			if (PlayerState && PlayerState->GetTeam())
			{
				const int32 TeamId = PlayerState->GetTeam()->TeamId;

				if (!TeamOverlaps.IsValidIndex(TeamId))
				{
					TeamOverlaps.SetNum(TeamId + 1, false);
				}

				check(TeamOverlaps[TeamId].Find(Character) == INDEX_NONE);
				TeamOverlaps[TeamId].Add(Character);
			}
		}

		UpdateControlState();
	}

	RecievedActivated();
}

void AAtomControlPoint::OnCapturedChanged_Implementation()
{
	if (ControllingTeam && bIsCaptured)
	{
		const FLinearColor Color = ControllingTeam->TeamColor;
		OutlineMesh->SetVectorParameterValueOnMaterials(FName{ TEXT("Color") }, FVector{ Color.R, Color.G, Color.B });
	}
	else
	{
		OutlineMesh->SetVectorParameterValueOnMaterials(FName{ TEXT("Color") }, FVector{ 1, 1, 1 });
	}	
}

void AAtomControlPoint::UpdateControlState()
{
	check(HasAuthority());

	// Get the state of the control point with overlapping teams. If more that one team is on the point, go to neutral.
	int32 OverlappingTeam = -1;
	for (int32 i = 0; i < TeamOverlaps.Num(); ++i)
	{
		if (TeamOverlaps[i].Num() > 0)
		{
			if (OverlappingTeam == -1)
			{
				OverlappingTeam = i;
			}
			else
			{
				OverlappingTeam = -1;
				break;
			}
		}
	}

	if (OverlappingTeam == -1)
	{
		SetControlState(EControlState::Contested);
	}
	else
	{
		check(TeamOverlaps.IsValidIndex(OverlappingTeam));

		if (ControllingTeam == nullptr)
		{
			// No controlling team? Set this one as the new one.
			if (auto GameState = GetWorld()->GetGameState<AAtomGameState>())
			{
				check(GameState->Teams.IsValidIndex(OverlappingTeam));
				ControllingTeam = GameState->Teams[OverlappingTeam];
				SetControlState(EControlState::Capturing);
			}			
		}
		else if (OverlappingTeam == ControllingTeam->TeamId)
		{
			// Only the controlling team is on the point
			SetControlState(EControlState::Capturing);
			
		}
		else
		{
			// Opposing teams are on the point
			SetControlState(EControlState::Lossing);
		}
	}
}

void AAtomControlPoint::SetControlState(const EControlState State)
{
	if (State == ControlState)
		return; 

	ControlState = State;

	// Send messages to only local players since this will be called on remotes too.
	if (State == EControlState::Capturing)
	{
		BroadcastTeamMessage(ControllingTeam, UAtomObjectiveMessage::EType::CapturingObjective);
	}
	else if (State == EControlState::Lossing)
	{
		BroadcastTeamMessage(ControllingTeam, UAtomObjectiveMessage::EType::LosingObjective);
	}
}

void AAtomControlPoint::BroadcastTeamMessage(AAtomTeamInfo* Team, const UAtomObjectiveMessage::EType Type)
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* Player = Iterator->Get();

		if (Player == nullptr)
			continue;

		auto PlayerState = Cast<AAtomPlayerState>(Player->PlayerState);
		if (PlayerState && PlayerState->GetTeam() == Team)
		{
			Player->ClientReceiveLocalizedMessage(ObjectiveMessageClass, UAtomObjectiveMessage::ConstructMessageIndex(Type, 3.f));
		}
	}
}
