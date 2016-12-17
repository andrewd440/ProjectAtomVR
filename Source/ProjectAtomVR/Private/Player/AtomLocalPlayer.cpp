// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomLocalPlayer.h"

void UAtomLocalPlayer::SetPlayerHeight(const float Height)
{
	PlayerHeight = Height;
}

float UAtomLocalPlayer::GetPlayerHeight() const
{
	return PlayerHeight;
}

void UAtomLocalPlayer::SetIsRightHanded(bool InbIsRightHanded)
{
	bIsRightHanded = InbIsRightHanded;
}

bool UAtomLocalPlayer::GetIsRightHanded() const
{
	return bIsRightHanded;
}
