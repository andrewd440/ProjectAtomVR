// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "VRHUD.generated.h"

enum class ELoadoutSlotChangeType : uint8;
class AAtomPlayerController;
class AAtomCharacter;

DECLARE_LOG_CATEGORY_EXTERN(LogVRHUD, Log, All);

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API AVRHUD : public AActor
{
	GENERATED_BODY()
	
protected:
	/** Help indicator that is pending creation from an initial delay. */
	struct FPendingHelpIndicator
	{
		FPendingHelpIndicator(FHelpIndicatorHandle InHelpHandle, const FText& InText,
			USceneComponent* InAttachParent, const FName InAttachSocket, const float InLifetime)
			: HelpHandle(InHelpHandle), Text(InText), AttachParent(InAttachParent),
			AttachSocket(InAttachSocket), LifeTime(InLifetime) {}

		FTimerHandle TimerHandle;
		FHelpIndicatorHandle HelpHandle;
		FText Text;
		USceneComponent* AttachParent;
		FName AttachSocket;
		float LifeTime;
	};

	/** Help indicator that has been spawned. */
	struct FActiveHelpIndicator
	{
		FActiveHelpIndicator(class AAtomFloatingText* InHelpActor, FHelpIndicatorHandle InHelpHandle)
			: Indicator(InHelpActor), HelpHandle(InHelpHandle) {}

		TWeakObjectPtr<class AAtomFloatingText> Indicator;
		FHelpIndicatorHandle HelpHandle;
	};

public:
	AVRHUD();

	AAtomPlayerController* GetPlayerController() const;

	AAtomCharacter* GetCharacter() const;

	/**
	* Called by the owning player controller when the possessed pawn is changed.
	*/
	void OnCharacterChanged(AAtomCharacter* OldCharacter);

	/** 
	 * Shows a help indicator. 
	 * 
	 * @return A unique handle for the indicator. 
	 */
	void ShowHelpIndicator(FHelpIndicatorHandle& Handle, const FText& Text, USceneComponent* AttachParent, 
		const FName AttachSocket, const float Lifetime, const float Delay);

	/** Clears an existing help indicator. This will clear active and pending indicators. */
	void ClearHelpIndicator(FHelpIndicatorHandle& Handle);

	/** AActor Interface Begin */
	virtual void SetOwner(AActor* NewOwner) override;
	virtual void Destroyed() override;
	virtual void Tick(float DeltaSeconds) override;
	/** AActor Interface End */

protected:
	/** Removes a pending help indicator from the list and creates an active indicator. */
	void CreatePendingHelpIndicator(const uint64 Handle);

	/** Spawns a help indicator and adds to the active indicator list. */
	void CreateActiveHelpIndicator(const FHelpIndicatorHandle& Handle, const FText& Text, USceneComponent* AttachParent,
		const FName AttachSocket, const float Lifetime);

	/**
	* Creates all UIs for the controlled character.
	*/
	void SpawnLoadoutActors();

	/**
	* Destroys all character UIs that have been created.
	*/
	void DestroyLoadoutActors(AAtomCharacter* OldCharacter);

private:
	void OnLoadoutSlotChanged(ELoadoutSlotChangeType Change, int32 LoadoutIndex);

protected:
	TArray<FPendingHelpIndicator> PendingHelpIndicators;

	TArray<FActiveHelpIndicator> ActiveHelpIndicators;

	uint32 bShowHelp : 1;

private:
	AAtomPlayerController* PlayerController = nullptr;

	uint64 NextHelpIndicatorHandle = 1;

	UPROPERTY()
	TArray<class AEquippableHUDActor*> LoadoutActors;			
};
