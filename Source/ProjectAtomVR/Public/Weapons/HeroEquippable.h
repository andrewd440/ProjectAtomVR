// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "HeroEquippable.generated.h"

UCLASS()
class PROJECTATOMVR_API AHeroEquippable : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHeroEquippable();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	virtual void Equip(const EHand Hand);

	virtual bool CanEquip(const EHand Hand) const;

	virtual void Unequip();

	void SetLoadoutAttachment(USceneComponent* AttachComponent, FName AttachSocket);

	bool IsEquipped() const;

	/** AActor Interface Begin */
public:
	virtual void SetOwner(AActor* NewOwner) override;
protected:
	virtual void OnRep_Owner() override;
	/** AActor Interface End */

protected:
	UPROPERTY(EditDefaultsOnly, Category = Equippable)
	FName HandAttachSocket = NAME_None;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Equippable, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* Mesh;

	class AHeroBase* HeroOwner = nullptr;

	/** Component that is attached to when unequipped */
	USceneComponent* StorageAttachComponent = nullptr;

	/** Socket that is attached to when unequipped */
	FName StorageAttachSocket = NAME_None;
};
