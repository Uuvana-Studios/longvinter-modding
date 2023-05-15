// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

[SupportedPlatforms(UnrealPlatformClass.Server)]
public class LongvinterServerTarget : TargetRules
{
	public LongvinterServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		BuildEnvironment = TargetBuildEnvironment.Unique;
		ExtraModuleNames.AddRange(new string[] { "Longvinter" });
		bUsesSteam = true;
		bUseLoggingInShipping = true;
		GlobalDefinitions.Add("UE4_PROJECT_STEAMGAMEDESC=\"Longvinter\"");
		GlobalDefinitions.Add("UE4_PROJECT_STEAMPRODUCTNAME=\"Longvinter\"");
		GlobalDefinitions.Add("UE4_PROJECT_STEAMGAMEDIR=\"Longvinter\"");
		GlobalDefinitions.Add("UE4_PROJECT_STEAMSHIPPINGID=1635450");
		ProjectDefinitions.Add("ONLINE_SUBSYSTEM_EOS_ENABLE_STEAM=1");
		DisablePlugins.Add("DiscordRichPresence");
	}
}
