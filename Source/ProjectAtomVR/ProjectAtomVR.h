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
#include "HeroBase.h"

/** Logging */
DECLARE_LOG_CATEGORY_EXTERN(AtomLog, Log, All); // A general log for your general needs

namespace AtomCollisionProfiles
{
	static const FName HeroHand{ TEXT("HeroHand") };
	static const FName HandTrigger{ TEXT("HandTrigger") };
}

#define INSTANTSHOT_CHANNEL ECC_GameTraceChannel3