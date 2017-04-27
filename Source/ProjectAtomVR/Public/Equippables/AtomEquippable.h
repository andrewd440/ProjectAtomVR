// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "AtomEquippable.generated.h"

class UEquippableState;

/**
 * Indicates the replication procedure used for un/equipping.
 */
UENUM()
enum class EEquipType : uint8
{
	Normal, // Immediately replicates action on un/equip
	Deferred // Allows un/equipping to be replicated outside calls to Equip/Unequip
};

/**
 * Represents the equip state of an Equippable.
 */
UENUM()
enum class EEquipState : uint8
{
	Unequipped, // In loadout
	Equipped, 
	Dropped
};

/**
 * Information describing the current equip status of a AtomEquippable.
 */
USTRUCT()
struct FEquipStatus
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
	EHand Hand;

	UPROPERTY(BlueprintReadOnly)
	EEquipState State;
	
	void ForceReplication()
	{
		++ForceRepCounter;
	}

private:
	UPROPERTY()
	uint32 ForceRepCounter : 1;
};

/**
 * Base class for all equippable items. 
 * Provides functionality for equipping, unequipping and dropping items, as well as management for states and transitions.
 */
UCLASS()
class PROJECTATOMVR_API AAtomEquippable : public AActor
{
	GENERATED_BODY()

protected:
	static const FName MeshComponentName;
	static const FName InactiveStateName;
	static const FName ActiveStateName;	

public:
	DECLARE_DELEGATE(FEquippedStatusChangedUI)
	FEquippedStatusChangedUI OnEquippedStatusChangedUI;

public:
	// Sets default values for this actor's properties
	AAtomEquippable(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	* Equips this to a specified hand of the owning character.
	* @param Hand The hand to equip to.
	* @param EquipType The equip replication type.
	*/
	virtual void Equip(const EHand Hand, const EEquipType EquipType = EEquipType::Normal);

	/**
	* Checks if this can equip to a specified hand of the owning character.
	* @param Hand The hand to check.
	* @returns True if this can be equipped.
	*/
	virtual bool CanEquip(const EHand Hand) const;

	/**
	* Equips this to a specified hand of the owning character.
	* @param EquipType The unequip replication type.
	*/
	virtual void Unequip(const EEquipType EquipType = EEquipType::Normal);

	/**
	* Drops this from the character's hand if equipped.
	*/
	virtual void Drop();

	/**
	* Updates the attach parent of the owning character. Used to update visuals when changing from first person to 
	* third person for players.
	*/
	virtual void UpdateCharacterAttachment();

	/** Checks if this is equipped. */
	bool IsEquipped() const;

	/** Gets the equipped hand, if equipped. */
	EHand GetEquippedHand() const;

	/** Gets the active state. */
	UEquippableState* GetCurrentState() const;

	/**
	 * Pops the current state.
	 * @param InPopState The state that should be popped. Only currently used as a sanity check to ensure stack validity.
	 **/
	void PopState(UEquippableState* InPopState);

	/**
	* Pushes a new state.
	* @param InPushState The state that should be pushed.
	**/
	void PushState(UEquippableState* InPushState);

	/**
	 * Called from ActiveState when entered.
	 */
	virtual void OnEquipped();

	/**
	* Called from InactiveState when entered.
	*/
	virtual void OnUnequipped();

	/** Checks if the character's secondary hand is attached to this. */
	bool IsSecondaryHandAttached() const;

	/** Gets the HUD type that represents this. */
	virtual TSubclassOf<class AEquippableHUDActor> GetHUDActor() const;

	/** Sets if attachment should be replicated. */
	void SetReplicatesAttachment(bool bShouldReplicate);

	/** Gets the type of loadout item this is. */
	ELoadoutType GetLoadoutType() const;

protected:
	/** Sets up input for this item from the owning character. */
	virtual void SetupInputComponent(UInputComponent* InputComponent);

	/**
	* Gets the SceneComponent that should be used when an offset needs to be applied to this object.
	* Useful for things such as recoil, which should be simulated by affecting this object.
	*/
	USceneComponent* GetOffsetTarget() const;

	
	/** Gets the original transform information for OffsetTarget. */
	void GetOriginalOffsetTargetLocationAndRotation(FVector& LocationOut, FRotator& RotationOut) const;

	UFUNCTION()
	virtual void OnBeginOverlapSecondaryHandTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
	virtual void OnEndOverlapSecondaryHandTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);	

private:
	UFUNCTION(Server, WithValidation, Reliable)
	void ServerEquip(const EHand Hand);

	UFUNCTION(Server, WithValidation, Reliable)
	void ServerUnequip();

	UFUNCTION(Server, WithValidation, Reliable)
	void ServerDrop();

	UFUNCTION(Server, WithValidation, Reliable)
	void ServerPushState(UEquippableState* State);

	UFUNCTION(Server, WithValidation, Reliable)
	void ServerPopState(UEquippableState* InPopState);

	UFUNCTION()
	void OnRep_EquipStatus();

	/** AActor Interface Begin */
public:
	virtual void BeginPlay() override;
	virtual void SetOwner(AActor* NewOwner) override;
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;
	virtual void GetSubobjectsWithStableNamesForNetworking(TArray<UObject*>& ObjList) override;
	virtual void PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker) override;
	virtual void Destroyed() override;

protected:
	virtual void OnRep_Owner() override;
	/** AActor Interface End */

protected:
	/** Socket to attach to when equipped. Depending on the hand equipped, this socket name will be post-fixed
	 ** with _l or _r.*/
	UPROPERTY(EditDefaultsOnly, Category = Equippable)
	FName PrimaryHandAttachSocket = NAME_None;

	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundBase* EquipSound = nullptr; // Played when equipped and unequipped

	/** Hand animation used when this item is equipped */
	UPROPERTY(EditDefaultsOnly, Category = Equippable)
	FHandAnim AnimHandEquip;

	/** Hand animation used when the secondary hand is attached */
	UPROPERTY(EditDefaultsOnly, Category = Equippable)
	FHandAnim AnimSecondaryHandEquip;

	UPROPERTY(Instanced, EditDefaultsOnly, BlueprintReadOnly, Category = States)
	UEquippableState* InactiveState;

	UPROPERTY(Instanced, EditDefaultsOnly, BlueprintReadOnly, Category = States)
	UEquippableState* ActiveState;

	TArray<UEquippableState*> StateStack;

	UPROPERTY(BlueprintReadOnly, Category = Equippable)
	FEquipStatus EquipStatus;

	UPROPERTY(ReplicatedUsing=OnRep_EquipStatus)
	FEquipStatus ReplicatedEquipStatus;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Equippable)
	TSubclassOf<class AEquippableHUDActor> HUDActor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Equippable)
	ELoadoutType LoadoutType;

	/** Seconds before this item can be equipped again after unequipping. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Equippable)
	float ReuseDelay = 0.f;

	/** Time stamp of when this item was last unequipped. */
	float UnequipTimeStamp = 0.f;

private:
	/** All Equippable states that support networking and are replicated. */
	TArray<UEquippableState*> EquippableStates;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Equippable, meta = (AllowPrivateAccess = "true"))
	class UMeshComponent* Mesh;

	UPROPERTY(BlueprintReadOnly, Category = Equippable, meta = (AllowPrivateAccess = "true"))
	class AAtomCharacter* CharacterOwner = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Equippable, meta = (AllowPrivateAccess = "true"))
	USphereComponent* SecondaryHandGripTrigger;

	uint32 bIsSecondaryHandAttached : 1;

	/** If true, when a unequipped hand overlaps SecondaryHandGripTrigger the hand can be attached to
	 ** SecondaryHandAttachLeft or SecondaryHandAttachRight sockets, depending on the hand.*/
	UPROPERTY(EditDefaultsOnly, Category = Equippable)
	uint32 bIsSecondaryHandAttachmentAllowed : 1;

	uint32 bReplicatesAttachment : 1;

	// True when this object is simulating events from replication.
	uint32 bIsSimulatingReplication : 1;

public:
	AAtomCharacter* GetCharacterOwner() const;

	template <typename MeshType = UMeshComponent>
	MeshType* GetMesh() const
	{
		return CastChecked<MeshType>(Mesh);
	}

protected:
	UEquippableState* GetInactiveState() const;
	UEquippableState* GetActiveState() const;
};

FORCEINLINE AAtomCharacter* AAtomEquippable::GetCharacterOwner() const { return CharacterOwner; }
FORCEINLINE UEquippableState* AAtomEquippable::GetInactiveState() const { return InactiveState; }
FORCEINLINE UEquippableState* AAtomEquippable::GetActiveState() const { return ActiveState; }
FORCEINLINE EHand AAtomEquippable::GetEquippedHand() const { return EquipStatus.Hand; }
FORCEINLINE bool AAtomEquippable::IsEquipped() const { return EquipStatus.State == EEquipState::Equipped; }
FORCEINLINE bool AAtomEquippable::IsSecondaryHandAttached() const { return bIsSecondaryHandAttached; }