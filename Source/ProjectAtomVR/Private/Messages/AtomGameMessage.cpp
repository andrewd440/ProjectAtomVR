// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomGameMessage.h"




UAtomGameMessage::UAtomGameMessage()
{
	RoundStartMessage = NSLOCTEXT("AtomGameMessage", "RoundStartMessage", "Round has started.");
	RoundNearEndMessage = NSLOCTEXT("AtomGameMessage", "RoundNearEndMessage", "Round is about to end.");
	RoundEndMessage = NSLOCTEXT("AtomGameMessage", "RoundEndMessage", "Round has ended.");
	GameEndMessage = NSLOCTEXT("AtomGameMessage", "GameEndMessage", "Game has ended.");
}
