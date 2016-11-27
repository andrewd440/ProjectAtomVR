// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "HeroEquippable.generated.h"

class UEquippableState;

UENUM()
enum class EEquipType : uint8
{
	Normal,
	Deferred
};

/**
 * Information describing the current equip status of HeroEquippable.
 */
USTRUCT()
struct FEquipStatus
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	EEquipType EquipType;

	UPROPERTY()
	EHand EquippedHand;

	UPROPERTY()
	uint32 bIsEquipped : 1;

	void ForceReplication()
	{
		++ForceRepCounter;
	}

private:
	UPROPERTY()
	uint32 ForceRepCounter : 1;
};

UCLASS()
class PROJECTATOMVR_API AHeroEquippable : public AActor
{
	GENERATED_BODY()

protected:
	static const FName MeshComponentName;
	static const FName InactiveStateName;
	static const FName ActiveStateName;	

public:
	/** Broadcasted when the return to loadout flag is changed for this Equippable. */
	DECLARE_EVENT(AHeroEquippable, FCanReturnToLoadoutChanged)
	FCanReturnToLoadoutChanged OnCanReturnToLoadoutChanged;

	DECLARE_DELEGATE(FEquippedStatusChangedUI)
	FEquippedStatusChangedUI OnEquippedStatusChangedUI;

public:
	// Sets default values for this actor's properties
	AHeroEquippable(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void Equip(const EHand Hand, const EEquipType EquipType = EEquipType::Normal);

	virtual bool CanEquip(const EHand Hand) const;

	virtual void Unequip(const EEquipType EquipType = EEquipType::Normal);

	/**
	* Control the behavior of this Equippable on Unequip. If true the Equippable will be returned to the
	* loadout when unequipped. If false, the Equippable will just be detached from the attached parent.
	*/
	void SetCanReturnToLoadout(bool bCanReturn);

	/**
	 * If this will be returned to the loadout on Unequip.
	 */
	bool CanReturnToLoadout() const;

	/**
	 * Sets the component to attach to on Unequip. Usually assigned by the HeroLoadout.
	 */
	void SetLoadoutAttachment(USceneComponent* AttachComponent, FName AttachSocket);

	bool IsEquipped() const;

	EHand GetEquippedHand() const;

	UEquippableState* GetCurrentState() const;

	void PopState(UEquippableState* InPopState);

	void PushState(UEquippableState* InPushState);

	/**
	 * Called from ActiveState when entered.
	 */
	virtual void OnEquipped();

	/**
	* Called from InactiveState when entered.
	*/
	virtual void OnUnequipped();

	bool IsSecondaryHandAttached() const;

	virtual TSubclassOf<class AEquippableUIActor> GetUIActor() const;

protected:
	virtual void SetupInputComponent(UInputComponent* InputComponent);

	void GetOriginalParentLocationAndRotation(FVector& LocationOut, FRotator& RotationOut) const;

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
	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void OnRep_Owner() override;
	/** AActor Interface End */

protected:
	/** Attach socket on the Equipped hand mesh to attach this mesh to. */
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

	UPROPERTY(ReplicatedUsing=OnRep_EquipStatus, BlueprintReadOnly, Category = Equippable)
	FEquipStatus EquipStatus;

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
	class AHeroBase* HeroOwner = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Equippable, meta = (AllowPrivateAccess = "true"))
	USphereComponent* SecondaryHandGripTrigger;

	/** Component that is attached to when unequipped */
	USceneComponent* LoadoutAttachComponent = nullptr;

	/** Socket that is attached to when unequipped */
	FName LoadoutAttachSocket = NAME_None;

	/** True the Equippable should be returned to the loadout when unequipped. If false,
	* the Equippable will just be detached from the attached parent. */
	uint32 bReturnToLoadout : 1;

	uint32 bIsSecondaryHandAttached : 1;

	/** If true, when a unequipped hand overlaps SecondaryHandGripTrigger the hand can be attached to
	 ** SecondaryHandAttachLeft or SecondaryHandAttachRight sockets, depending on the hand.*/
	UPROPERTY(EditDefaultsOnly, Category = Equippable)
	uint32 bIsSecondaryHandAttachmentAllowed : 1;

public:
	AHeroBase* GetHeroOwner() const;

	template <typename MeshType = UMeshComponent>
	MeshType* GetMesh() const
	{
		check(MeshType::StaticClass()->IsChildOf(Mesh->StaticClass()) && "Mesh Component is not of the requested type.");
		return static_cast<MeshType*>(Mesh);
	}

protected:
	UEquippableState* GetInactiveState() const;
	UEquippableState* GetActiveState() const;
};

FORCEINLINE AHeroBase* AHeroEquippable::GetHeroOwner() const { return HeroOwner; }
FORCEINLINE UEquippableState* AHeroEquippable::GetInactiveState() const { return InactiveState; }
FORCEINLINE UEquippableState* AHeroEquippable::GetActiveState() const { return ActiveState; }
FORCEINLINE EHand AHeroEquippable::GetEquippedHand() const { return EquipStatus.EquippedHand; }
FORCEINLINE bool AHeroEquippable::IsEquipped() const { return EquipStatus.bIsEquipped; }
FORCEINLINE bool AHeroEquippable::IsSecondaryHandAttached() const { return bIsSecondaryHandAttached; }