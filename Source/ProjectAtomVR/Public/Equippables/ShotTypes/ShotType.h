// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Object.h"
#include "ShotType.generated.h"

//-------------------------------------------------------------------------------------
// Parameters used to gather and use shot data by UShotType when firing a shot 
// and replicating data to the server, when invoked by a client.
//-------------------------------------------------------------------------------------
USTRUCT()
struct FShotData
{
	GENERATED_USTRUCT_BODY()

	// The start location of the shot
	UPROPERTY()
	FVector_NetQuantize10 Start;

	// The end location of the shot
	UPROPERTY()
	FVector_NetQuantize10 End;
};

/**
* Base for any type of shot that can be fired from a firearm.
* Provides an interface for getting the initial shot data for a shot, simulating a shot, 
* and firing a shot on the server.
*/
UCLASS(Blueprintable, Abstract, NotPlaceable, Config = Game, DefaultToInstanced, EditInlineNew)
class PROJECTATOMVR_API UShotType : public UObject
{
	GENERATED_BODY()

public:

	/**
	* [Client/Server]
	* Gets shot data associated with a shot being fired from the owning weapon.
	*
	* @param ShotData	Output of the shot data.
	*
	* @returns True if a valid shot data could be retrieved. If false, the shot should
	*			not be invoked.
	*/
	UFUNCTION(BlueprintCallable, Category = ShotType)
	virtual FShotData GetShotData() const PURE_VIRTUAL(UBSShotType::GetShotData, return FShotData{};);

	/**
	* [Client]
	* Used to simulate a shot by playing effects solely on the connection that is provoking the shot.
	* To be used when a cached version of the shot data is available, such as after calling GetShotData.
	*
	* @param ShotData The shot data produced by GetShotData.
	*/
	UFUNCTION(BlueprintCallable, Category = ShotType)
	virtual void SimulateShot(const FShotData& ShotData);

	/**
	* [Server]
	* Fires the actual shot. Deals damage, replicates events to remotes, verifies hits, etc.
	*
	* @param ShotData	The shot data provided by GetShotData().
	* @returns
	*/
	UFUNCTION(BlueprintCallable, Category = ShotType)
	virtual void FireShot(const FShotData& ShotData) PURE_VIRTUAL(UBSShotType::InvokeShot, );

	/** UObject Interface Begin */
	virtual class UWorld* GetWorld() const override;
	/** UObject Interface End */

protected:
	/** The owning weapon */
	class AHeroFirearm* GetFirearm() const;
};