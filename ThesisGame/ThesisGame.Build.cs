// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ThesisGame : ModuleRules
{
	public ThesisGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		//PrivatePCHHeaderFile = "ThesisGame.h";

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "UMG" });
		//PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "UMG" });

		PublicIncludePaths.AddRange(
			new string[] {
				"ThesisGame",
				"ThesisGame/Public/Activators",
				"ThesisGame/Public/Characters",
				"ThesisGame/Public/Pickups",
				"ThesisGame/Public/Quests",
				"ThesisGame/Public/Weapons",
			}
		);
        PrivateIncludePaths.AddRange(
            new string[] {
				"ThesisGame",
				"ThesisGame/Private/Activators",
				"ThesisGame/Private/Characters",
				"ThesisGame/Private/Pickups",
				"ThesisGame/Private/Quests",
				"ThesisGame/Private/Weapons",
			}
        );

        PublicDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "CoreUObject",
                "Engine",
                "OnlineSubsystem",
                "OnlineSubsystemUtils",
                "AssetRegistry",
                "NavigationSystem",
                "AIModule",
                "GameplayTasks",
                "Gauntlet",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "InputCore",
                "Slate",
                "SlateCore",
                "Json",
                "ApplicationCore",
                "ReplicationGraph",
                "PakFile",
                "RHI"
            }
        );
    }
}
