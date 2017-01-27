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

	UPROPERTY(EditAnywhere)
	int32 MaxPlayers;

	UPROPERTY(EditAnywhere)
	int32 TimeLimit;

	UPROPERTY(EditAnywhere)
	int32 ScoreLimit;

	UPROPERTY(EditAnywhere)
	int32 TeamCount;

	UPROPERTY(EditAnywhere)
	TArray<FLinearColor> TeamColors;
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
	
	const FPlaylistItem& CyclePlaylist();

	const FPlaylistItem& CurrentItem() const;

private:
	UPROPERTY(EditAnywhere, Category = Playlist)
	TArray<FPlaylistItem> Playlist;

	int32 PlaylistIndex = -1;
};
