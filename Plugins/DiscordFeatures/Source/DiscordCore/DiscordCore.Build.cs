// Copyright Pandores Marketplace 2021. All Rights Reserved.

using System;
using UnrealBuildTool;
using System.IO;

public class DiscordCore : ModuleRules
{
	public DiscordCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));

		SetDiscordFunctionality("Achievement",	true);
		SetDiscordFunctionality("Activity",		true);
		SetDiscordFunctionality("Application",	true);
		SetDiscordFunctionality("Image",		true);
		SetDiscordFunctionality("Lobby",		true);
		SetDiscordFunctionality("Voice",		true);
		SetDiscordFunctionality("Network",		true);
		SetDiscordFunctionality("Overlay",		true);
		SetDiscordFunctionality("Storage",		true);
		SetDiscordFunctionality("Store",		true);
		SetDiscordFunctionality("Relationship",	true);
		SetDiscordFunctionality("User",			true);

		PrivateDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });
		PublicDependencyModuleNames .AddRange(new string[] { "DiscordSdk" });
	}

	void SetDiscordFunctionality(string FunctionalityName, bool bEnabled)
	{
		PublicDefinitions.Add("DISCORD_WITH_" + FunctionalityName.ToUpper() + "=" + (bEnabled ? "1" : "0"));
	}
}
