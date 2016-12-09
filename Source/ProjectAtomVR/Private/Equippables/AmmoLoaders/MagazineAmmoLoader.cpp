// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "MagazineAmmoLoader.h"
#include "HeroFirearm.h"
#include "FirearmMagazine.h"

namespace
{
	static const FName MagazineAttachSocket{ TEXT("MagazineAttach") };

	static constexpr float SmoothLoadSpeed = 30.f;
	static constexpr float ClipAttachRotationErrorDegrees = 20.f;
	static constexpr float ClipAttachRotationErrorRadians = ClipAttachRotationErrorDegrees * (PI / 180.f);
}

UMagazineAmmoLoader::UMagazineAmmoLoader(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	bRequiresEquippedClip = true;
	bIsLoadingMagazine = false;
	bHasInitialMagazine = false;

	ReloadTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("MagazineReloadTrigger"));	
	ReloadTrigger->SetIsReplicated(false);
	ReloadTrigger->SetSphereRadius(2.f);
	ReloadTrigger->bGenerateOverlapEvents = false;
	ReloadTrigger->SetCollisionObjectType(CollisionChannelAliases::FirearmReloadTrigger);
	ReloadTrigger->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	ReloadTrigger->SetCollisionResponseToChannel(CollisionChannelAliases::ClipLoadTrigger, ECollisionResponse::ECR_Overlap);

	ReloadTrigger->OnComponentBeginOverlap.AddDynamic(this, &UMagazineAmmoLoader::OnMagazineEnteredReloadTrigger);		
}

void UMagazineAmmoLoader::OnEquipped()
{
	if (Magazine == nullptr && GetFirearm()->GetHeroOwner()->IsLocallyControlled())
	{
		ReloadTrigger->bGenerateOverlapEvents = true;
	}	
}

void UMagazineAmmoLoader::OnUnequipped()
{
	ReloadTrigger->bGenerateOverlapEvents = false;
}

void UMagazineAmmoLoader::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UMagazineAmmoLoader, Magazine, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UMagazineAmmoLoader, RemoteConnectionMagazine, COND_OwnerOnly);
}

UWorld* UMagazineAmmoLoader::GetTickableGameObjectWorld() const
{
	return GetWorld();
}

bool UMagazineAmmoLoader::IsTickable() const
{
	return	GetFirearm() != nullptr &&
			GetFirearm()->IsEquipped() &&
			bIsLoadingMagazine;
}

void UMagazineAmmoLoader::BeginPlay()
{
	Super::BeginPlay();
}

TStatId UMagazineAmmoLoader::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UMagazineAmmoLoader, STATGROUP_Tickables);
}

void UMagazineAmmoLoader::Tick(float DeltaTime)
{
	if (bIsLoadingMagazine)
	{
		UMeshComponent* const MagazineMesh = Magazine->GetMesh();

		const FVector NewLocation = FMath::VInterpConstantTo(MagazineMesh->RelativeLocation, FVector::ZeroVector, DeltaTime, SmoothLoadSpeed);
		const FRotator NewRotation = FMath::RInterpConstantTo(MagazineMesh->RelativeRotation, FRotator::ZeroRotator, DeltaTime, SmoothLoadSpeed);

		if (NewLocation.IsNearlyZero() && NewRotation.IsNearlyZero())
		{
			MagazineMesh->SetRelativeLocationAndRotation(FVector::ZeroVector, FQuat::Identity);
			bIsLoadingMagazine = false;
			AmmoCount = Magazine->GetCapacity();

			OnAmmoCountChanged.ExecuteIfBound();
		}
		else
		{
			MagazineMesh->SetRelativeLocationAndRotation(NewLocation, NewRotation);
		}
	}	
}

void UMagazineAmmoLoader::InitializeLoader()
{
	Super::InitializeLoader();

	if (GetFirearm()->HasAuthority() && MagazineTemplate)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetFirearm()->GetHeroOwner();
		
		const FTransform Transform = GetFirearm()->GetMesh()->GetSocketTransform(MagazineAttachSocket); // Spawn at attach location
		RemoteConnectionMagazine = GetWorld()->SpawnActor<AFirearmMagazine>(MagazineTemplate, Transform, SpawnParams);
		LoadAmmo(RemoteConnectionMagazine);
	}

	if (!GetFirearm()->IsTemplate())
	{
		ReloadTrigger->AttachToComponent(GetFirearm()->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform);
	}
}

bool UMagazineAmmoLoader::DiscardAmmo()
{
	if (!Magazine)
	{
		return false;
	}

	Magazine->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform); 

	Magazine->bTearOff = true;
	Magazine->GetLoadTrigger()->bGenerateOverlapEvents = false; // Disable trigger on eject, pending destroy

	UMeshComponent* const MagazineMesh = Magazine->GetMesh();
	MagazineMesh->SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);
	Magazine->SetActorEnableCollision(true);

	MagazineMesh->SetSimulatePhysics(true);

	Magazine->SetLifeSpan(10.f);

	bIsLoadingMagazine = false;
	Magazine = nullptr;
	AmmoCount = 0;

	ReloadTrigger->bGenerateOverlapEvents = true;

	OnAmmoCountChanged.ExecuteIfBound();

	return true;
}

void UMagazineAmmoLoader::LoadAmmo(UObject* LoadObject)
{
	Super::LoadAmmo(LoadObject);

	ensure(Magazine == nullptr);
	check(Cast<AFirearmMagazine>(LoadObject));

	Magazine = static_cast<AFirearmMagazine*>(LoadObject);

	ensureMsgf(Magazine->IsA(MagazineTemplate),
		TEXT("Attached HeroFirearm magazine is not compatible with assigned type. Was %s, while MagazineTemplate is %s"), Magazine->StaticClass()->GetName(), MagazineTemplate->StaticClass()->GetName());

	ReloadTrigger->bGenerateOverlapEvents = false; // Disable immediately to prevent newly spawned loadout items from trying to load ammo.

	Magazine->SetCanReturnToLoadout(false);

	if (Magazine->IsEquipped())
	{
		Magazine->Unequip(EEquipType::Deferred);
	}

	Magazine->SetActorEnableCollision(false);
	Magazine->GetMesh()->SetSimulatePhysics(false);	

	if (bHasInitialMagazine)
	{
		Magazine->AttachToComponent(GetFirearm()->GetMesh(), FAttachmentTransformRules::KeepWorldTransform, MagazineAttachSocket);
		bIsLoadingMagazine = true;
	}
	else
	{
		Magazine->AttachToComponent(GetFirearm()->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, MagazineAttachSocket);
		AmmoCount = Magazine->GetCapacity();
		bHasInitialMagazine = true;

		OnAmmoCountChanged.ExecuteIfBound();
	}
}

void UMagazineAmmoLoader::SetupInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupInputComponent(InputComponent);

	const FName EjectActionName = (GetFirearm()->GetEquippedHand() == EHand::Left) ? TEXT("EjectClipLeft") : TEXT("EjectClipRight");
	InputComponent->BindAction(EjectActionName, IE_Pressed, this, &UMagazineAmmoLoader::OnMagazineEjectPressed);
}

void UMagazineAmmoLoader::OnMagazineEnteredReloadTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	check(Cast<AFirearmMagazine>(OtherActor) && "FirearmMagazine should be the only response to this trigger.");
	check(GetFirearm()->IsEquipped() && "Overlap events should be unbound when not the HeroFirearm is not equipped.");

	AFirearmMagazine* OverlapMagazine = static_cast<AFirearmMagazine*>(OtherActor);
	AHeroFirearm* MyFirearm = GetFirearm();

	if (OverlapMagazine->GetHeroOwner() == MyFirearm->GetHeroOwner() &&
		(!bRequiresEquippedClip || OverlapMagazine->IsEquipped()) &&
		OverlapMagazine->IsA(MagazineTemplate))
	{
		FQuat AttachRotation = MyFirearm->GetMesh<UMeshComponent>()->GetSocketQuaternion(MagazineAttachSocket);
		FQuat ClipRotation = OverlapMagazine->GetActorQuat();

		if (AttachRotation.AngularDistance(ClipRotation) <= ClipAttachRotationErrorRadians)
		{
			// Align magazine and load into firearm
			OverlapMagazine->SetActorLocationAndRotation(OverlappedComponent->GetComponentLocation(), ClipRotation);
			GetFirearm()->LoadAmmo(OverlapMagazine);
		}
	}
}

void UMagazineAmmoLoader::OnRep_DefaultMagazine()
{
	if (RemoteConnectionMagazine)
	{
		LoadAmmo(RemoteConnectionMagazine);
	}
}

void UMagazineAmmoLoader::OnRep_Magazine()
{
	if (Magazine == nullptr)
	{
		Magazine = RemoteConnectionMagazine;
		GetFirearm()->DiscardAmmo();
	}
	else
	{
		// Simulate loading a new magazine
		AFirearmMagazine* NewMagazine = Magazine;
		Magazine = nullptr;
		GetFirearm()->LoadAmmo(NewMagazine);
	}

	RemoteConnectionMagazine = Magazine;
}

void UMagazineAmmoLoader::OnMagazineEjectPressed()
{
	GetFirearm()->DiscardAmmo();
}
