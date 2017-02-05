// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProjectAtomVR : ModuleRules
{
	public ProjectAtomVR(TargetInfo Target)
	{
        PrivateIncludePaths.AddRange(
            new string[] {
                "ProjectAtomVR/Private",
                "ProjectAtomVR/Private/Character",
                "ProjectAtomVR/Private/Equippables",
                "ProjectAtomVR/Private/GameModes",
                "ProjectAtomVR/Private/MotionComponents",
                "ProjectAtomVR/Private/Online",
                "ProjectAtomVR/Private/Player",
            });

        PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
			});

		PrivateDependencyModuleNames.AddRange(
			new string[] {
                "OnlineSubsystem",
                "OnlineSubsystemUtils",
                "Slate",
                "SlateCore",
				"HeadMountedDisplay",
				"UMG",
            });


		DynamicallyLoadedModuleNames.AddRange(
			new string[] {
				"OnlineSubsystemNull",
				"OnlineSubsystemSteam",
			});
	}
}
