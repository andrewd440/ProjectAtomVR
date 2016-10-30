// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "HeroEquippable.h"
#include "ShotTypes/ShotType.h"
#include "HeroFirearm.generated.h"

class UEquippableState;
enum class EFirearmNotify : uint8;

USTRUCT()
struct FFirearmStats
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, Category = Stats)
	float Damage;

	UPROPERTY(EditDefaultsOnly, Category = Stats)
	float FireRate; // Seconds between shots

	UPROPERTY(EditDefaultsOnly, Category = Stats)
	uint32 MaxAmmo;

	UPROPERTY(EditDefaultsOnly, Category = Stats)
	uint32 ClipSize;
};

UCLASS(Abstract)
class PROJECTATOMVR_API AHeroFirearm : public AHeroEquippable
{
	GENERATED_BODY()
	
public:
	/** Broadcasted when a clip has been attached or ejected. */
	DECLARE_EVENT(AHeroFirearm, FOnClipChanged)
	FOnClipChanged OnClipChanged;

public:	
	AHeroFirearm(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void Tick( float DeltaSeconds ) override;

	/** Get the location of the firearm muzzle in world space. */
	FVector GetMuzzleLocation() const;

	/** Get the rotation of the firearm muzzle in world space. */
	FQuat GetMuzzleRotation() const;

	int32 GetRemainingAmmo() const;

	int32 GetRemainingClip() const;

	class AFirearmClip* GetClip() const;

	TSubclassOf<AFirearmClip> GetClipClass() const;

	virtual void AttachClip(class AFirearmClip* Clip);

	virtual void EjectClip();

	void ConsumeAmmo();

	const FFirearmStats& GetFirearmStats() const;

	void FireShot();

	virtual void StartFiringSequence();

	virtual void StopFiringSequence();

	virtual void OnFirearmNotify(EFirearmNotify Type);

	//bool IsBoltPullNeeded() const;

	//void SetNeedsBoltPull(bool bIsNeeded);

	UEquippableState* GetFiringState() const;
	UEquippableState* GetChargingState() const;
	UEquippableState* GetReloadingState() const;

	USkeletalMeshComponent* GetSkeletalMesh() const;

	/** 
	 * Gets trigger used to determine valid overlap for a clip to be loaded.
	 */
	UShapeComponent* GetClipReloadTrigger() const;

	const FName GetClipAttachSocket() const;

	/** AHeroEquippable Interface Begin */
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	/** AHeroEquippable Interface End */

protected:
	virtual void PlaySingleShotSequence();
	
	UFUNCTION()
	virtual void OnRep_CurrentClip();

private:
	UFUNCTION(Server, WithValidation, Reliable)
	void ServerFireShot(FShotData ShotData);

	UFUNCTION(Server, WithValidation, Reliable)
	void ServerAttachClip(class AFirearmClip* Clip);

	UFUNCTION(Server, WithValidation, Reliable)
	void ServerEjectClip();

	UFUNCTION()
	void OnRep_DefaultClip();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Firearm)
	FFirearmStats Stats;

	int32 RemainingAmmo;

	int32 RemainingClip;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Firearm)
	TSubclassOf<class AFirearmClip> ClipClass;

	UPROPERTY(Transient, ReplicatedUsing=OnRep_CurrentClip, BlueprintReadOnly, Category = Firearm)
	AFirearmClip* CurrentClip;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Firearm)
	FName ClipAttachSocket;

	UPROPERTY(Instanced, EditDefaultsOnly, BlueprintReadOnly, Category = Firearm)
	class UShotType* ShotType;

	UPROPERTY(Instanced, EditDefaultsOnly, BlueprintReadOnly, Category = States)
	UEquippableState* FiringState;

	UPROPERTY(Instanced, EditDefaultsOnly, BlueprintReadOnly, Category = States)
	UEquippableState* ChargingState;

	UPROPERTY(Instanced, EditDefaultsOnly, BlueprintReadOnly, Category = States)
	UEquippableState* ReloadingState;

	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Firearm)
	//FName BoltCarrierGripSocket;

	//UPROPERTY(EditDefaultsOnly, Category = Firearm)
	//TArray<FVector> BoltCarrierTargets;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Firearm)
	FName ShellEjectSocket;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Firearm)
	UParticleSystem* ShellEjectTemplate = nullptr;

	UPROPERTY(transient)
	UParticleSystemComponent* ShellEjectComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Firearm)
	FName MuzzleSocket;

	// Spawned particle system component for muzzle FX
	UPROPERTY(Transient)
	UParticleSystemComponent* MuzzleFXComponent = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Firearm)
	UParticleSystem* MuzzleFX = nullptr;

	// Spawned component used to play FireSound and manipulate looping sounds.
	UPROPERTY(Transient)
	UAudioComponent* FireSoundComponent = nullptr;

	// Sound effect when the weapon is fire. This audio will loop while in the
	// firing weapon state if it is a looping sound. Otherwise, it will be played
	// at the time of each shot. For automatic weapons, it is recommended to use
	// looping firing sounds.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Sound)
	USoundBase* FireSound = nullptr;

	// Sound effect on weapon end fire
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Sound)
	USoundBase* EndFireSound = nullptr;

	// Sound effect on weapon fire with empty clip
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Sound)
	USoundBase* EmptyClipSound = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Animation)
	UAnimMontage* BoltCarrierMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Animation)
	UAnimMontage* TriggerPullMontage = nullptr;

	/** True if the bolt carrier should be automatically reset if the grip is let go. */
	//UPROPERTY(EditDefaultsOnly, Category = Firearm)
	//uint32 bAutoResetBoltCarrier : 1;

	// Is a bolt pull currently needed
	uint32 bNeedsBoltPull : 1;	

private:
	/** Clip that is added to the firearm when spawned for owning clients. Is also used to save
	 ** a copy of the current clip to have a reference to the old clip once overwritten from replication. */
	UPROPERTY(ReplicatedUsing = OnRep_DefaultClip)
	AFirearmClip* RemoteConnectionClip = nullptr;

	/** Trigger used to determine valid overlap for a clip to be loaded.*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Firearm, meta = (AllowPrivateAccess = "true"))
	UShapeComponent* ClipReloadTrigger;

};

FORCEINLINE UEquippableState* AHeroFirearm::GetFiringState() const { return FiringState; }

FORCEINLINE UEquippableState* AHeroFirearm::GetChargingState() const { return ChargingState; }

FORCEINLINE UEquippableState* AHeroFirearm::GetReloadingState() const { return ReloadingState; }

FORCEINLINE int32 AHeroFirearm::GetRemainingAmmo() const { return RemainingAmmo; }

FORCEINLINE int32 AHeroFirearm::GetRemainingClip() const { return RemainingClip; }

FORCEINLINE const FFirearmStats& AHeroFirearm::GetFirearmStats() const { return Stats; }

FORCEINLINE USkeletalMeshComponent* AHeroFirearm::GetSkeletalMesh() const { return static_cast<USkeletalMeshComponent*>(GetMesh()); }

FORCEINLINE class AFirearmClip* AHeroFirearm::GetClip() const { return CurrentClip; }

FORCEINLINE TSubclassOf<AFirearmClip> AHeroFirearm::GetClipClass() const { return ClipClass; }

FORCEINLINE UShapeComponent* AHeroFirearm::GetClipReloadTrigger() const { return ClipReloadTrigger; }

FORCEINLINE const FName AHeroFirearm::GetClipAttachSocket() const { return ClipAttachSocket; }

//FORCEINLINE bool AHeroFirearm::IsBoltPullNeeded() const { return bNeedsBoltPull; }
//FORCEINLINE void AHeroFirearm::SetNeedsBoltPull(bool bIsNeeded) { bIsNeeded = bIsNeeded; }