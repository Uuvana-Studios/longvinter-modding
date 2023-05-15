// Copyright Pandores Marketplace 2021. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class DiscordAchievementEditor: ModuleRules
{
	public DiscordAchievementEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]{ "Core" });

		PrivateDependencyModuleNames.AddRange(new string[]{ 
			"CoreUObject", 
			"Engine", 
			"HTTP", 
			"Slate", 
			"SlateCore", 
			"DiscordSdk", 
			"Json",
			"DiscordSdkEditor",
			"MainFrame",
			"DesktopPlatform"
		});

		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
	}
}

