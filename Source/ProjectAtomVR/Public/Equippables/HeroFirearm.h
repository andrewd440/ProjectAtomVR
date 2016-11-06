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
	uint32 MagazineSize;

	UPROPERTY(EditDefaultsOnly, Category = Stats)
	uint32 bHasSlideLock : 1;
};

/**
 * Shots are fired from the socket name "Muzzle" on the firearm mesh.
 */
UCLASS(Abstract)
class PROJECTATOMVR_API AHeroFirearm : public AHeroEquippable
{
	GENERATED_BODY()
	
public:
	/** Broadcasted when a magazine has been attached or ejected. */
	DECLARE_EVENT(AHeroFirearm, FOnClipChanged)
	FOnClipChanged OnMagazineChanged;

public:	
	AHeroFirearm(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void Tick( float DeltaSeconds ) override;

	/** Get the location of the firearm muzzle in world space. */
	FVector GetMuzzleLocation() const;

	/** Get the rotation of the firearm muzzle in world space. */
	FQuat GetMuzzleRotation() const;

	int32 GetRemainingAmmo() const;

	int32 GetRemainingMagazine() const;

	bool IsChamberEmpty() const;

	UFUNCTION(BlueprintCallable, Category = Firearm)
	float GetChamberingProgress() const;

	UFUNCTION(BlueprintCallable, Category = Firearm)
	bool IsHoldingChamberingHandle() const;

	bool CanFire() const;

	class AFirearmClip* GetMagazine() const;

	TSubclassOf<AFirearmClip> GetMagazineClass() const;

	virtual void InsertMagazine(class AFirearmClip* Clip);

	virtual void EjectMagazine();

	const FFirearmStats& GetFirearmStats() const;

	void FireShot();

	virtual void DryFire();

	virtual void StartFiringSequence();

	virtual void StopFiringSequence();

	virtual void OnFirearmAnimNotify(EFirearmNotify Type);

	UEquippableState* GetFiringState() const;
	UEquippableState* GetChargingState() const;
	UEquippableState* GetReloadingState() const;

	/** 
	 * Gets trigger used to determine valid overlap for a magazine to be loaded.
	 */
	UShapeComponent* GetMagazineReloadTrigger() const;
	const FName GetMagazineAttachSocket() const;

protected:
	virtual void PlaySingleShotSequence();
	
	virtual bool ShouldDisableTick() const;

	virtual void OnOppositeHandTriggerPressed();
	virtual void OnOppositeHandTriggerReleased();

	bool CanGripChamberingHandle() const;
	void OnChamberingHandleReleased();
	void OnSlideLockPressed();
	void ActivateSlideLock();
	void ReleaseSlideLock();

	/**
	* Ejects a cartridge.
	* 
	* @param bIsFired True if the cartridge is fired.
	*/
	void ReloadChamber(bool bIsFired);

	UFUNCTION()
	virtual void OnEjectedCartridgeCollide(FName EventName, float EmitterTime, int32 ParticleTime, FVector Location, FVector Velocity, FVector Direction, FVector Normal, FName BoneName, UPhysicalMaterial* PhysMat);

	UFUNCTION()
	virtual void OnRep_CurrentMagazine();

private:
	UFUNCTION(Server, WithValidation, Reliable)
	void ServerFireShot(FShotData ShotData);

	UFUNCTION(Server, WithValidation, Reliable)
	void ServerInsertMagazine(class AFirearmClip* Clip);

	UFUNCTION(Server, WithValidation, Reliable)
	void ServerEjectMagazine();

	UFUNCTION()
	void OnRep_DefaultMagazine();

	/** AHeroEquippable Interface Begin */
public:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
protected:
	virtual void SetupInputComponent(UInputComponent* InputComponent) override;
	/** AHeroEquippable Interface End */

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Firearm)
	FFirearmStats Stats;

	int32 RemainingAmmo;

	int32 RemainingMagazine;

	/** The type of magazine this firearm uses. Magazines will be attached to the "MagazineAttach" socket.*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Firearm)
	TSubclassOf<class AFirearmClip> MagazineClass;

	UPROPERTY(Transient, ReplicatedUsing=OnRep_CurrentMagazine, BlueprintReadOnly, Category = Firearm)
	AFirearmClip* CurrentMagazine;

	/**
	* Shots type for this firearm. Shots are fired from the socket name "Muzzle" on the mesh.
	*/
	UPROPERTY(Instanced, EditDefaultsOnly, BlueprintReadOnly, Category = Firearm)
	class UShotType* ShotType;

	UPROPERTY(Instanced, EditDefaultsOnly, BlueprintReadOnly, Category = States)
	UEquippableState* FiringState;

	UPROPERTY(Instanced, EditDefaultsOnly, BlueprintReadOnly, Category = States)
	UEquippableState* ChargingState;

	UPROPERTY(Instanced, EditDefaultsOnly, BlueprintReadOnly, Category = States)
	UEquippableState* ReloadingState;

	/** 
	 * Particle system used to eject shells from the firearm. The emitter named "CartridgeFired" will be
	 * used when ejecting a fired cartridge and "CartridgeUnfired" will be used when ejecting a unfired cartridge. 
	 * This system will be attached to the "CartridgeAttach" socket.
	 * */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Firearm)
	UParticleSystem* CartridgeEjectTemplate = nullptr;

	UPROPERTY(transient)
	UParticleSystemComponent* CartridgeEjectComponent = nullptr;

	/** 
	 * Radius the hand "Grip" socket is required to be within to grab the chamber handle. The chambering handle location
	 * is the location of the "ChamberingHandle" socket. 
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Firearm)
	float ChamberingHandleRadius;

	/** List of movement vectors required to reset the chamber. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Firearm)
	TArray<FVector> ChamberHandleMovement;

	/** Mesh attached to "CartridgeAttach" socket for fired cartridges */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Firearm)
	UStaticMesh* CartridgeFiredMesh;

	/** Mesh attached to "CartridgeAttach" socket for unfired cartridges */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Firearm)
	UStaticMesh* CartridgeUnfiredMesh;

	UPROPERTY(transient)
	UStaticMeshComponent* CartridgeMeshComponent = nullptr;

	/** Spawned particle system component for muzzle FX */
	UPROPERTY(Transient)
	UParticleSystemComponent* MuzzleFXComponent = nullptr;

	/** Particle system component for muzzle FX. Will be attached to the "Muzzle" socket. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Firearm)
	UParticleSystem* MuzzleFX = nullptr;

	// Spawned component used to play FireSound and manipulate looping sounds.
	UPROPERTY(Transient)
	UAudioComponent* FireSoundComponent = nullptr;

	// Sound effect when the weapon is fire. This audio will loop while in the
	// firing weapon state if it is a looping sound. Otherwise, it will be played
	// at the time of each shot. For automatic weapons, it is recommended to use
	// looping firing sounds with the SoundNodeFadeOut used for the tail sound.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Sound)
	USoundBase* FireSound = nullptr;

	/** Sound effect on weapon fire with empty magazine */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Sound)
	USoundBase* DryFireSound = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Sound)
	USoundBase* MagazineInsertSound = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Sound)
	USoundBase* MagazineEjectSound = nullptr;

	/** Sound played when an ejected cartridge hits a surface. The Cartridge Eject Template must generate
	 ** collision event for this sound to play.*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Sound)
	USoundBase* CartridgeCollideSound = nullptr;

	/** Sounds to played at each stage during ChamberHandleMovement. Sounds that correspond ChamberHandleMovement
	 * going forward should be at the first ChamberHandleMovement count indices. Sounds that should be played when
	 * reversing ChamberHandleMovement should be at the second half of this list. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Sound)
	TArray<USoundBase*> ChamberingSounds;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Animation)
	UAnimMontage* FiringMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Animation)
	UAnimMontage* TriggerPullMontage = nullptr;

private:
	/** Magazine that is added to the firearm when spawned for owning clients. Is also used to save
	 ** a copy of the current magazine to have a reference to the old magazine once overwritten from replication. */
	UPROPERTY(ReplicatedUsing = OnRep_DefaultMagazine)
	AFirearmClip* RemoteConnectionMagazine = nullptr;

	/** Trigger used to determine valid overlap for a magazine to be loaded.*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Firearm, meta = (AllowPrivateAccess = "true"))
	UShapeComponent* MagazineReloadTrigger;

	/** The origin hand location at the start of the current chambering index. This is relative to the actor. */
	FVector ChamberingHandStartLocation = FVector::ZeroVector;

	/** State of the interactive chambering process. Only valid while chambering handle is being held. */
	float ChamberingProgress = 0.f;
	uint8 ChamberingIndex = 0;

	uint32 bIsChamberEmpty : 1;
	uint32 bIsHoldingChamberHandle : 1;

	enum class EChamberState : uint8
	{
		Set,
		Unset, // In pulled position
	};

	EChamberState LastChamberState : 1;
	uint32 bIsSlideLockActive : 1;
};

FORCEINLINE UEquippableState* AHeroFirearm::GetFiringState() const { return FiringState; }

FORCEINLINE UEquippableState* AHeroFirearm::GetChargingState() const { return ChargingState; }

FORCEINLINE UEquippableState* AHeroFirearm::GetReloadingState() const { return ReloadingState; }

FORCEINLINE int32 AHeroFirearm::GetRemainingAmmo() const { return RemainingAmmo; }

FORCEINLINE int32 AHeroFirearm::GetRemainingMagazine() const { return RemainingMagazine; }

FORCEINLINE const FFirearmStats& AHeroFirearm::GetFirearmStats() const { return Stats; }

FORCEINLINE class AFirearmClip* AHeroFirearm::GetMagazine() const { return CurrentMagazine; }

FORCEINLINE TSubclassOf<AFirearmClip> AHeroFirearm::GetMagazineClass() const { return MagazineClass; }

FORCEINLINE UShapeComponent* AHeroFirearm::GetMagazineReloadTrigger() const { return MagazineReloadTrigger; }