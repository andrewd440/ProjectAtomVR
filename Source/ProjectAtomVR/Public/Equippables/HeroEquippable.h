// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "HeroEquippable.generated.h"

class UEquippableState;

/**
 * Information describing the current equip status of HeroEquippable.
 */
USTRUCT()
struct FEquipStatus
{
	GENERATED_USTRUCT_BODY()

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
	
public:	
	// Sets default values for this actor's properties
	AHeroEquippable(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	virtual void Equip(const EHand Hand);

	virtual bool CanEquip(const EHand Hand) const;

	virtual void Unequip();

	void SetStorageAttachment(USceneComponent* AttachComponent, FName AttachSocket);

	bool IsEquipped() const;

	void PushActiveState();

	void PopState(UEquippableState* InPopState);

	/**
	 * Called from ActiveState when entered.
	 */
	virtual void OnEquipped();

	/**
	* Called from InactiveState when entered.
	*/
	virtual void OnUnequipped();

protected:
	void PushState(UEquippableState* InPushState);

	UFUNCTION()
	void OnRep_EquipStatus();

private:
	UFUNCTION(Server, WithValidation, Reliable)
	void ServerEquip(const EHand Hand);

	UFUNCTION(Server, WithValidation, Reliable)
	void ServerUnequip();

	/** AActor Interface Begin */
public:
	virtual void SetOwner(AActor* NewOwner) override;
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void OnRep_Owner() override;
	/** AActor Interface End */

protected:
	UPROPERTY(EditDefaultsOnly, Category = Equippable)
	FName HandAttachSocket = NAME_None;

	/** Hand animation used when this item is equipped */
	UPROPERTY(EditDefaultsOnly, Category = Equippable)
	UAnimSequence* AnimHandEquip = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = Equippable)
	TSubclassOf<UEquippableState> InactiveStateTemplate = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = Equippable)
	TSubclassOf<UEquippableState> ActiveStateTemplate = nullptr;

	TArray<UEquippableState*> StateStack;

	UPROPERTY(ReplicatedUsing=OnRep_EquipStatus, BlueprintReadOnly, Category = Equippable)
	FEquipStatus EquipStatus;

	/** Seconds before this item can be equipped again after unequipping. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Equippable)
	float ReuseDelay = 0.f;

	/** Time stamp of when this item was last unequipped. */
	float UnequipTimeStamp = 0.f;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Equippable, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* Mesh;

	UPROPERTY(BlueprintReadOnly, Category = Equippable, meta = (AllowPrivateAccess = "true"))
	class AHeroBase* HeroOwner = nullptr;

	/** Component that is attached to when unequipped */
	USceneComponent* StorageAttachComponent = nullptr;

	/** Socket that is attached to when unequipped */
	FName StorageAttachSocket = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = Equippable, meta = (AllowPrivateAccess = "true"))
	UEquippableState* InactiveState = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = Equippable, meta = (AllowPrivateAccess = "true"))
	UEquippableState* ActiveState = nullptr;

public:
	AHeroBase* GetHeroOwner() const;

protected:
	UEquippableState* GetInactiveState() const;
	UEquippableState* GetActiveState() const;
};

FORCEINLINE AHeroBase* AHeroEquippable::GetHeroOwner() const { return HeroOwner; }
FORCEINLINE UEquippableState* AHeroEquippable::GetInactiveState() const { return InactiveState; }
FORCEINLINE UEquippableState* AHeroEquippable::GetActiveState() const { return ActiveState; }