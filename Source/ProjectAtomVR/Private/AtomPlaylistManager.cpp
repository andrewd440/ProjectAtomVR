// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomPlaylistManager.h"




UAtomPlaylistManager::UAtomPlaylistManager()
{

}

FPlaylistItem UAtomPlaylistManager::CyclePlaylist()
{
	FPlaylistItem CycledItem;

	if (Playlist.Num() > 0)
	{
		CycledItem = Playlist[0];

		Playlist.RemoveAt(0, 1, false);
		Playlist.Add(CycledItem);
	}

	return CycledItem;
}
