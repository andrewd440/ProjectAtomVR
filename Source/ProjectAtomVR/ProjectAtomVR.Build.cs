// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProjectAtomVR : ModuleRules
{
	public ProjectAtomVR(TargetInfo Target)
	{
        PrivateIncludePaths.AddRange(
            new string[] {
                        "ProjectAtomVR/Private",
                        "ProjectAtomVR/Private/Heroes",
                        "ProjectAtomVR/Private/Heroes/Movement",
            });

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

		PrivateDependencyModuleNames.AddRange(new string[] { "HeadMountedDisplay" });

		// Uncomment if you are using Slate UI
		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
