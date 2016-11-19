// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Equippables/AmmoLoaders/AmmoLoader.h"
#include "MagazineAmmoLoader.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API UMagazineAmmoLoader : public UAmmoLoader
{
	GENERATED_BODY()
	
public:
	UMagazineAmmoLoader(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** UAmmoLoader Interface Begin */
	virtual void OnEquipped() override;
	virtual void OnUnequipped() override;
	virtual void SetupInputComponent(class UInputComponent* InputComponent) override;
	virtual bool DiscardAmmo() override;
	virtual void LoadAmmo(UObject* LoadObject) override;
	virtual void InitializeLoader() override;
	virtual bool IsTickable() const override;
	virtual void Tick(float DeltaTime) override;
	/** UAmmoLoader Interface End */

	/** UObject Interface Begin */
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> & OutLifetimeProps) const override;	
	/** UObject Interface End */

protected:
	UFUNCTION()
	virtual void OnMagazineEnteredReloadTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnRep_DefaultMagazine();

	UFUNCTION()
	void OnRep_Magazine();

	void OnMagazineEjectPressed();

protected:
	/** The type of magazine this firearm uses. Magazines will be attached to the "MagazineAttach" socket on the owning HeroFirearm mesh.*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = MagazineAmmoLoader)
	TSubclassOf<class AFirearmMagazine> MagazineTemplate;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_Magazine, BlueprintReadOnly, Category = MagazineAmmoLoader)
	AFirearmMagazine* Magazine;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = MagazineAmmoLoader)
	FVector TriggerRelativeOffset = FVector::ZeroVector;

private:
	/** Magazine that is added to the firearm when spawned for owning clients. Is also used to save
	** a copy of the current magazine to have a reference to the old magazine once overwritten from replication. */
	UPROPERTY(ReplicatedUsing = OnRep_DefaultMagazine)
	AFirearmMagazine* RemoteConnectionMagazine = nullptr;

	/** Trigger used to determine valid overlap for a magazine to be loaded.*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = MagazineAmmoLoader, meta = (AllowPrivateAccess = "true"))
	USphereComponent* ReloadTrigger;

protected:
	/** Does the clip need to be in hand to load into the firearm. */
	UPROPERTY(EditDefaultsOnly, Category = MagazineAmmoLoader)
	uint32 bRequiresEquippedClip : 1;

private:
	uint32 bIsLoadingMagazine : 1;
};
