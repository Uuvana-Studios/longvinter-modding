// Copyright Epic Games, Inc. All Rights Reserved.
using System.IO;
using UnrealBuildTool;

public class Longvinter : ModuleRules
{
    public Longvinter(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core", 
            "CoreUObject", 
            "Engine", 
            "InputCore", 
            "HeadMountedDisplay", 
            "OnlineSubsystem", 
            "OnlineSubsystemUtils", 
            "Steamworks",
            "Slate",
            "SlateCore",
            "GamepadUMGPlugin",
			"SteamCore",
            "ProceduralDungeon",
            "PakFile",
            "XmlParser",
            "HTTP",
			"Sockets",
            "Networking"      
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            
        });

        DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
	
    }
}
