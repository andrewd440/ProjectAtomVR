// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "GameModes/AtomGameObjective.h"
#include "AtomControlPoint.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API AAtomControlPoint : public AAtomGameObjective
{
	GENERATED_BODY()
		
public:
	DECLARE_EVENT(AAtomControlPoint, FOnCaptured)

public:
	AAtomControlPoint();

	/** Gets the bounds used to capture the point. */
	UBoxComponent* GetCaptureBounds() const;

	UStaticMeshComponent* GetOutlineMesh() const;

	/** Activates the control point with an optional delay before actual activation. */
	void Activate(const float Delay = 0.f);

	/** True if this point is active. */
	bool IsActive();

	/** 
	 * Gets the controlling team. The controlling team does not always have the point captured. 
	 * This is the team that is influencing the current Control on the point. 
	 */
	AAtomTeamInfo* GetControllingTeam() const;

	/** True if the controlling team has the point captured. */
	bool IsCaptured() const;
	
	/** Broadcasted when the point is captured by a team. */
	FOnCaptured& OnCaptured() { return OnCapturedEvent; }

	/** Gets the members of the controlling team that are within the capture bounds. */
	TArray<AAtomPlayerState*> GetActiveControllingTeamMembers() const;

	/** AAtomGameObjective Interface Begin */
	virtual void InitializeObjective() override;
	/** AAtomGameObjective Interface End */

	/** AActor Interface Begin */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaSeconds) override;
	/** AActor Interface End */

protected:
	UFUNCTION()
	void OnRep_ActivationTimestamp();

	UFUNCTION()
	void OnRep_IsActive();

	UFUNCTION()
	void OnRep_IsCaptured();

	UFUNCTION()
	void OnBoundsBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnBoundsEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void OnActivated();

	UFUNCTION(BlueprintImplementableEvent)
	void RecievedActivated();

	UFUNCTION(BlueprintNativeEvent)
	void OnCapturedChanged();

	void UpdateControlState();

protected:
	UPROPERTY(BlueprintReadWrite, Replicated, Transient, Category = ControlPoint)
	AAtomTeamInfo* ControllingTeam = nullptr; // The team in control of the point.

	UPROPERTY(BlueprintReadOnly, Replicated, Transient, Category = ControlPoint)
	float Control = 0.f; // Control the controlling team has over the point. [0-1.0]

	UPROPERTY(EditDefaultsOnly, Category = ControlPoint, meta = (UIMin="0.0", UIMax="1.0"))
	float ControlRate = 0.05f; // Rate per second that control is transferred for the point.

	// Factor of ControlRate that is added for each additional team member capturing the point, up to MaxTeamInfluence.
	UPROPERTY(EditDefaultsOnly, Category = ControlPoint, meta = (UIMin = "0.0"))
	float ControlRateMultiplier = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = ControlPoint, meta = (UIMin="1"))
	int32 MaxTeamInfluence = 3; // Max team members that can influence the capture rate.

	// Indexed by team id, the number of players inside the bounds of the point.
	TArray<int32> OverlapTeamCounts;

	FOnCaptured OnCapturedEvent;

	enum class EControlState : uint8
	{
		Lossing,
		Neutral,
		Capturing
	};

	EControlState ControlState = EControlState::Neutral;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ControlPoint, meta = (AllowPrivateAccess="true"))
	UBoxComponent* CaptureBounds;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ControlPoint, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* OutlineMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ControlPoint, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* InnerSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ControlPoint, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* OuterSphere;

	UPROPERTY(ReplicatedUsing = OnRep_ActivationTimestamp)
	float ActivationTimestamp;

	UPROPERTY(ReplicatedUsing = OnRep_IsActive)
	uint32 bIsActive : 1;

	UPROPERTY(ReplicatedUsing = OnRep_IsCaptured)
	uint32 bIsCaptured : 1;
};

FORCEINLINE UBoxComponent* AAtomControlPoint::GetCaptureBounds() const { return CaptureBounds; }

FORCEINLINE UStaticMeshComponent* AAtomControlPoint::GetOutlineMesh() const { return OutlineMesh; }
