// Copyright Pandores Marketplace 2021. All Rights Reserved.

using UnrealBuildTool;
using System.IO;
using System;

public class DiscordSdkEditor: ModuleRules
{
	public DiscordSdkEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]{ "Core" });

		PrivateDependencyModuleNames.AddRange(new string[]{ 
			"CoreUObject",
			"DiscordSdk",
			"DiscordCore",
			"Projects",
			"InputCore",
			"UnrealEd",
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
			"MainFrame",
			"HTTP",
			"zlib"
		});

#if UE_4_24_OR_LATER
		PrivateDependencyModuleNames.AddRange(new string[]{ "ToolMenus" });
#else
		PrivateDependencyModuleNames.AddRange(new string[] { "LevelEditor" });
#endif

#if UE_5_0_OR_LATER
		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "../ThirdParty/"));
#else
		PrivateIncludePaths.Add(Path.Combine(EngineDirectory, "Source/" + Target.UEThirdPartySourceDirectory + "zlib/zlib-1.2.5/Src/Contrib/"));
#endif

		PublicIncludePaths .Add(Path.Combine(ModuleDirectory, "Public"));
#if UE_5_0_OR_LATER
		PublicDefinitions.Add("DISCORD_FEATURES_EDITOR_UE_5_0_OR_LATER=1");
#else
		PublicDefinitions.Add("DISCORD_FEATURES_EDITOR_UE_5_0_OR_LATER=0");
#endif
	}
}

