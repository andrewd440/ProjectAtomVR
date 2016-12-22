// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "UObject/NoExportTypes.h"
#include "AtomPlaylistManager.generated.h"

USTRUCT()
struct FPlaylistItem
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	FName MapName;

	UPROPERTY(EditAnywhere)
	FName GameMode;

	UPROPERTY(EditAnywhere)
	int32 MinPlayers;
};

/**
 * 
 */
UCLASS(Blueprintable)
class PROJECTATOMVR_API UAtomPlaylistManager : public UObject
{
	GENERATED_BODY()
	
public:
	UAtomPlaylistManager();
	
	FPlaylistItem CyclePlaylist();
	
private:

	UPROPERTY(EditAnywhere, Category = Playlist)
	TArray<FPlaylistItem> Playlist;
};
