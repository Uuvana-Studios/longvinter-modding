// Copyright Pandores Marketplace 2021. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class DiscordNetwork: ModuleRules
{
	public DiscordNetwork(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]{ "Core" });

		PrivateDependencyModuleNames.AddRange(new string[]{ "CoreUObject", "Engine", "DiscordCore", "DiscordSdk" });

		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
	}
}

