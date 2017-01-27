// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomPlaylistManager.h"




UAtomPlaylistManager::UAtomPlaylistManager()
{

}

const FPlaylistItem& UAtomPlaylistManager::CyclePlaylist()
{
	PlaylistIndex = Playlist.IsValidIndex(PlaylistIndex + 1) ? PlaylistIndex + 1 : 0;
	return Playlist[PlaylistIndex];
}

const FPlaylistItem& UAtomPlaylistManager::CurrentItem() const
{
	return Playlist[PlaylistIndex];
}
