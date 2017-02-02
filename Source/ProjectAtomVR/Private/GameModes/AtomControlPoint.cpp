// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomControlPoint.h"
#include "AtomCharacter.h"
#include "AtomPlayerState.h"
#include "AtomTeamInfo.h"




AAtomControlPoint::AAtomControlPoint()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.TickInterval = 1.f; // Only needed every second. Used to update TeamControl.

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

	OverlapTeamCounts.SetNum(2); // Most, if not all, games will have 2 teams
	bIsActive = false;
	bIsCaptured = false;
}

void AAtomControlPoint::Activate(const float Delay/* = 0.f*/)
{
	if (Delay > 0)
	{
		FTimerHandle DiscardHandle;
		GetWorldTimerManager().SetTimer(DiscardHandle, this, &AAtomControlPoint::OnActivated, Delay);
	}
	else
	{
		OnActivated();
	}
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
	TArray<AActor*> InitalOverlaps;
	CaptureBounds->GetOverlappingActors(InitalOverlaps, AAtomCharacter::StaticClass());

	for (AActor* InitialOverlap : InitalOverlaps)
	{
		auto PlayerState = Cast<AAtomPlayerState>(static_cast<AAtomCharacter*>(InitialOverlap)->PlayerState);

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

	// Modify TeamControl based on ControlState
	if (ControlState == EControlState::Capturing)
	{
		// Get overlapping team count
		const int32 TeamInfluence = FMath::Min(MaxTeamInfluence, OverlapTeamCounts[ControllingTeam->TeamId]);
		const float Rate = ControlRate + (TeamInfluence - 1) * ControlRateMultiplier * ControlRate;
		ensure(Rate > 0);

		Control = FMath::Min(1.f, Control + Rate * DeltaSeconds);
		
		if (HasAuthority() && Control == 1.f && !bIsCaptured)
		{
			bIsCaptured = true;
			OnCapturedChanged();
			OnCapturedEvent.Broadcast();
		}
	}
	else if (ControlState == EControlState::Lossing)
	{
		// Get overlapping enemy count
		int32 TeamInfluence = 0;
		for (auto TeamCount : OverlapTeamCounts)
		{
			TeamInfluence += TeamCount;
		}

		TeamInfluence -= OverlapTeamCounts[ControllingTeam->TeamId];
		TeamInfluence = FMath::Min(MaxTeamInfluence, TeamInfluence);

		const float Rate = ControlRate + (TeamInfluence - 1) * ControlRateMultiplier * ControlRate;
		ensure(Rate > 0);

		Control = FMath::Max(0.f, Control - Rate * DeltaSeconds);

		if (HasAuthority() && Control == 0.f && bIsCaptured)
		{
			bIsCaptured = false;
			ControllingTeam = nullptr;
			UpdateControlState();
			OnCapturedChanged();
		}
	}
}

void AAtomControlPoint::OnRep_ActivationTimestamp()
{

}

void AAtomControlPoint::OnRep_IsActive()
{
	OnActivated();
}

void AAtomControlPoint::OnRep_IsCaptured()
{
	UpdateControlState();
	OnCapturedChanged();

	if (bIsCaptured)
	{
		OnCapturedEvent.Broadcast();
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

				if (!OverlapTeamCounts.IsValidIndex(TeamId))
				{
					OverlapTeamCounts.SetNum(TeamId + 1, false);
				}

				++OverlapTeamCounts[TeamId];
				UpdateControlState();
			}
		}
	}
}

void AAtomControlPoint::OnBoundsEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ensureMsgf(Cast<AAtomCharacter>(OtherActor) != nullptr, TEXT("Control points should only respond to AtomCharacter overlaps."));

	if (auto Character = Cast<AAtomCharacter>(OtherActor))
	{
		if (auto PlayerState = Cast<AAtomPlayerState>(Character->PlayerState))
		{
			if (AAtomTeamInfo* Team = PlayerState->GetTeam())
			{
				const int32 TeamId = Team->TeamId;

				check(OverlapTeamCounts.IsValidIndex(TeamId));
				--OverlapTeamCounts[TeamId];
				UpdateControlState();
			}
		}
	}
}

void AAtomControlPoint::OnActivated()
{
	bIsActive = true;
	CaptureBounds->bGenerateOverlapEvents = true; // Activate overlaps

	// Get initial overlaps
	TArray<AActor*> InitalOverlaps;
	CaptureBounds->GetOverlappingActors(InitalOverlaps, AAtomCharacter::StaticClass());

	for (AActor* InitialOverlap : InitalOverlaps)
	{
		auto PlayerState = Cast<AAtomPlayerState>(static_cast<AAtomCharacter*>(InitialOverlap)->PlayerState);

		if (PlayerState && PlayerState->GetTeam())
		{
			const int32 TeamId = PlayerState->GetTeam()->TeamId;

			if (!OverlapTeamCounts.IsValidIndex(TeamId))
			{
				OverlapTeamCounts.SetNum(TeamId + 1, false);
			}

			++OverlapTeamCounts[TeamId];			
		}
	}

	UpdateControlState();
	RecievedActivated();
}

void AAtomControlPoint::OnCapturedChanged_Implementation()
{
	
}

void AAtomControlPoint::UpdateControlState()
{
	// Get the state of the control point with overlapping teams. If more that one team is on the point, go to neutral.
	int32 OverlappingTeam = -1;
	for (int32 i = 0; i < OverlapTeamCounts.Num(); ++i)
	{
		if (OverlapTeamCounts[i] > 0)
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
		ControlState = EControlState::Neutral;
	}
	else
	{
		check(OverlapTeamCounts.IsValidIndex(OverlappingTeam));

		if (ControllingTeam == nullptr)
		{
			// No controlling team? Set this one as the new one.
			if (auto GameState = GetWorld()->GetGameState<AAtomGameState>())
			{
				check(GameState->Teams.IsValidIndex(OverlappingTeam));
				ControllingTeam = GameState->Teams[OverlappingTeam];
				ControlState = EControlState::Capturing;
			}			
		}
		else if (OverlappingTeam == ControllingTeam->TeamId)
		{
			// Only the controlling team is on the point
			ControlState = EControlState::Capturing;
		}
		else
		{
			// Opposing teams are on the point
			ControlState = EControlState::Lossing;
		}
	}
}
