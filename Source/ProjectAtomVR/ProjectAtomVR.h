// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Engine.h"
#include "UnrealNetwork.h"

/** VR Includes */
#include "IHeadMountedDisplay.h"
#include "MotionControllerComponent.h"

/** Online Includes */
#include "OnlineSubsystem.h"

/** Gameplay Includes */
#include "AtomTypes.h"
#include "AtomCharacter.h"
#include "AtomPlayerController.h"
#include "AtomGameMode.h"
#include "AtomGameState.h"

/** Logging */
DECLARE_LOG_CATEGORY_EXTERN(LogAtom, Log, All); // A general log for your general needs
DECLARE_LOG_CATEGORY_EXTERN(LogAtomOnlineSession, Log, All);

namespace AtomCollisionProfiles
{
	static const FName HeroHand{ TEXT("HeroHand") };
	static const FName HandTrigger{ TEXT("HandTrigger") };
}

namespace AtomCollisionChannels
{
	static constexpr ECollisionChannel HeroHand = ECC_GameTraceChannel1;
	static constexpr ECollisionChannel HandTrigger = ECC_GameTraceChannel2;
	static constexpr ECollisionChannel InstantShot = ECC_GameTraceChannel3;
	static constexpr ECollisionChannel ClipLoadTrigger = ECC_GameTraceChannel4;
	static constexpr ECollisionChannel FirearmReloadTrigger = ECC_GameTraceChannel5;
}