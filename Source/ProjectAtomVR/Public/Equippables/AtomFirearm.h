// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "AtomEquippable.h"
#include "ShotTypes/ShotType.h"
#include "AtomFirearm.generated.h"

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

	// Determines how fast weapon recoil offsets are reset after firing the weapon.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Accuracy)
	float Stability;

	/** The average direction that recoil from each shot will push the weapon.
	 ** This on the x plane, so X = right and Y = up */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Recoil)
	FVector2D RecoilRotationalPush;

	/** Direction in the XZ plane that a directional offset is applied due to recoil. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Recoil)
	FVector2D RecoilDirectionalPush;

	/** Value used to damped recoil velocity and directional offset. This is also applied to 
	 ** recoil angular velocity input when a secondary hand is attached. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Recoil)
	float RecoilDampening;

	/** Angle, in degrees, of recoil spread for the weapon. This is the max angle that RecoilRotationalPush
	 ** will be offset when calculating the final recoil push direction for each shot. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Recoil)
	float RecoilPushSpread;

	UPROPERTY(EditDefaultsOnly, Category = Stats)
	uint32 bHasSlideLock : 1;
};

/**
* Structure used to represent the volume used to check if a firearm is
* in a valid position to fire.
*/
USTRUCT()
struct FFirearmBlockFireVolume
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly)
	FVector RelativePosition = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly)
	FQuat RelativeRotation = FQuat::Identity;

	UPROPERTY(EditDefaultsOnly)
	float CapsuleRadius = 0.f;

	UPROPERTY(EditDefaultsOnly)
	float CapsuleHalfHeight = 0.f;
};

/**
 * Class for all firearm type equippable items.
 * Shots are fired from the socket name "Muzzle" on the firearm mesh.
 */
UCLASS(Abstract)
class PROJECTATOMVR_API AAtomFirearm : public AAtomEquippable
{
	GENERATED_BODY()

protected:
	enum class EHelpIndicatorType : uint8
	{
		LoadAmmo,
		ReleaseSlideLock,
		ChamberAmmo,
		MAX_COUNT
	};

public:	
	AAtomFirearm(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void Tick( float DeltaSeconds ) override;	

	/** Get the location of the firearm muzzle in world space. */
	FVector GetMuzzleLocation() const;

	/** Get the rotation of the firearm muzzle in world space. */
	FQuat GetMuzzleRotation() const;

	UFUNCTION(BlueprintCallable, Category = Firearm)
	bool IsChamberEmpty() const;

	/** Gets the progress of the chambering position of the firearm. [0-1] */
	UFUNCTION(BlueprintCallable, Category = Firearm)	
	float GetChamberingProgress() const;

	/** Checks if the chambering handle is held. */
	UFUNCTION(BlueprintCallable, Category = Firearm)
	bool IsHoldingChamberingHandle() const;

	UFUNCTION(BlueprintCallable, Category = Firearm)
	bool IsSlideLockActive() const;

	bool CanFire() const;

	bool IsMuzzleInGeometry() const;

	/** Loads ammo for the firearm. Usually only used by the active ammo loader for the firearm for RPC support. */
	void LoadAmmo(UObject* LoadObject, bool bForceLocalOnly = false);

	/** Discards ammo for the firearm, if supported by the active ammo loader. */
	virtual void DiscardAmmo();

	const FFirearmStats& GetFirearmStats() const;

	/** Fires a shot based on the active shot type. */
	void FireShot();

	/** Dry fires the firearm. Plays audio effect. */
	virtual void DryFire();

	/**
	* Starts the sequence for entering a firing state. (i.e. plays trigger pull anim)
	*/
	virtual void StartFiringSequence();

	/**
	* Stops all active firing sequences. (i.e. muzzle effects, firing sounds, etc.)
	*/
	virtual void StopFiringSequence();

	UEquippableState* GetFiringState() const;
	UEquippableState* GetChargingState() const;

	UAmmoLoader* GetAmmoLoader() const;

	/** 
	 * Activates the slide lock. 
	 * The assigned FiringMontage must have a SlideLock section. 
	 */
	void ActivateSlideLock();

protected:
	/** Updates the positioning and progress of the chambering handle when held. */
	void UpdateChamberingHandle();

	/** Updates the recoil offset of the firearm each frame recoil is active. */
	void UpdateRecoilOffset(float DeltaSeconds);

	/** Plays all effects for a single shot. */
	virtual void PlaySingleShotSequence();

	virtual void OnOppositeHandTriggerPressed();
	virtual void OnOppositeHandTriggerReleased();

	/** Checks if the owning character's hand is in a position to grip the chambering handle. */
	bool CanGripChamberingHandle() const;

	void OnChamberingHandleGrabbed();
	void OnChamberingHandleReleased();

	void OnSlideLockPressed();
	void ReleaseSlideLock();

	/** Generates active shot recoil based on an input seed. Generated recoil is directly applied to RecoilVelocity. */
	void GenerateShotRecoil(uint8 Seed);

	void ShowHelp(EHelpIndicatorType Type, const float Lifetime);
	void ClearHelp(EHelpIndicatorType Type);

	/**
	* Ejects a cartridge and reloads the chamber.
	* 
	* @param bIsFired True if the cartridge is fired.
	*/
	void ReloadChamber(bool bIsFired);

	/** Callback for cartridge eject particle system to play collision audio. */
	UFUNCTION()
	void OnEjectedCartridgeCollide(FName EventName, float EmitterTime, int32 ParticleTime, FVector Location, FVector Velocity, 
		FVector Direction, FVector Normal, FName BoneName, UPhysicalMaterial* PhysMat);

	UFUNCTION()
	void OnRep_IsHoldingChamberHandle();

	UFUNCTION()
	void OnRep_IsSlideLockActive();

private:
	UFUNCTION(Server, WithValidation, Reliable)
	void ServerFireShot(FShotData ShotData);

	UFUNCTION(Server, WithValidation, Reliable)
	void ServerLoadAmmo(UObject* LoadObject);

	UFUNCTION(Server, WithValidation, Reliable)
	void ServerDiscardAmmo();

	UFUNCTION(Server, WithValidation, Reliable)
	void ServerSetSlideLock(bool bIsActive);

	UFUNCTION(Server, WithValidation, Reliable)
	void ServerSetIsHoldingChamberingHandle(bool bIsHeld);

	/** AHeroEquippable Interface Begin */
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;
	virtual void OnEquipped() override;
	virtual void OnUnequipped() override;
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
protected:
	virtual void SetupInputComponent(UInputComponent* InputComponent) override;
	/** AHeroEquippable Interface End */

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Firearm)
	FFirearmStats Stats;

	/**
	* Structure used to represent the volume used to check if a firearm is
	* in a valid position to fire.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Firearm)
	FFirearmBlockFireVolume BlockFireVolume;

	struct FRecoilVelocity
	{
		FVector Angular;
		FVector Directional;
	} RecoilVelocity;

	UPROPERTY(Instanced, EditDefaultsOnly, BlueprintReadOnly, Category = Firearm)
	class UAmmoLoader* AmmoLoader = nullptr;

	/**
	* Shots type for this firearm. Shots are fired from the socket name "Muzzle" on the mesh.
	*/
	UPROPERTY(Instanced, EditDefaultsOnly, BlueprintReadOnly, Category = Firearm)
	class UShotType* ShotType = nullptr;

	UPROPERTY(Instanced, EditDefaultsOnly, BlueprintReadOnly, Category = States)
	UEquippableState* FiringState;

	UPROPERTY(Instanced, EditDefaultsOnly, BlueprintReadOnly, Category = States)
	UEquippableState* ChargingState = nullptr;
	
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
	UStaticMesh* CartridgeFiredMesh = nullptr;

	/** Mesh attached to "CartridgeAttach" socket for unfired cartridges */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Firearm)
	UStaticMesh* CartridgeUnfiredMesh = nullptr;

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
	USoundBase* LoadAmmoSound = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Sound)
	USoundBase* DiscardAmmoSound = nullptr;

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
	/** The origin hand location at the start of the current chambering index. This is relative to the actor. */
	FVector ChamberingHandStartLocation = FVector::ZeroVector;

	/** State of the interactive chambering process. Only valid while chambering handle is being held. */
	float ChamberingProgress = 0.f;

	/** Current index in ChamberHandleMovement we are at. Only valid when chambering handle is held. */
	uint8 ChamberingIndex = 0;

	UPROPERTY(Replicated)
	uint32 bIsChamberEmpty : 1;

	UPROPERTY(ReplicatedUsing=OnRep_IsHoldingChamberHandle)
	uint32 bIsHoldingChamberHandle : 1;

	enum class EChamberState : uint8
	{
		Set, // In start position
		Unset, // In pulled position
	};

	/** The last full chambering state we were in. Used to determine if we are moving forward or backwards in
	 ** ChamberHandleMovement. */
	EChamberState LastChamberState : 1;

	UPROPERTY(ReplicatedUsing = OnRep_IsSlideLockActive)
	uint32 bIsSlideLockActive : 1;

	/** True when there is active recoil and needs to return to the original location/rotation */
	uint32 bIsRecoilActive : 1;

	FHelpIndicatorHandle HelpHandles[static_cast<uint8>(EHelpIndicatorType::MAX_COUNT)];
};

FORCEINLINE UEquippableState* AAtomFirearm::GetFiringState() const { return FiringState; }

FORCEINLINE UEquippableState* AAtomFirearm::GetChargingState() const { return ChargingState; }

FORCEINLINE const FFirearmStats& AAtomFirearm::GetFirearmStats() const { return Stats; }