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
			"OnlineSubsystemRedpointEOS",
			"RedpointEOSSDK",
            "OnlineSubsystemUtils", 
            "Steamworks",
            "Slate",
            "SlateCore",
            "GamepadUMGPlugin",
            "ProceduralDungeon"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {

        });

        DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
		
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "../../Plugins/EOSOnlineSubsystem/Source/ThirdParty/Include"));
		
		if (Target.Platform == UnrealTargetPlatform.Win64) {
			
        PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "../../Plugins/EOSOnlineSubsystem/Source/ThirdParty/Lib/EOSSDK-Win64-Shipping.lib"));
		
	   }
	   if (Target.Platform == UnrealTargetPlatform.Linux) {
		   
	   PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "../../Plugins/EOSOnlineSubsystem/Source/ThirdParty/Bin/libEOSSDK-Linux-Shipping.so"));
	   }
    }
}
