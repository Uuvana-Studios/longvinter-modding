// Copyright Pandores Marketplace 2021. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class DiscordActivity: ModuleRules
{
	public DiscordActivity(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]{ "Core" });

		PrivateDependencyModuleNames.AddRange(new string[]{ "CoreUObject", "Engine", "DiscordCore" });

		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
	}
}

