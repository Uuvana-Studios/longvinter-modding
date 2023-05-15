// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class LongvinterTarget : TargetRules
{
	public LongvinterTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		BuildEnvironment = TargetBuildEnvironment.Unique;
		ExtraModuleNames.Add("Longvinter");
		bUsesSteam = true;
		bUseLoggingInShipping = true;
		GlobalDefinitions.Add("UE4_PROJECT_STEAMGAMEDESC=\"Longvinter\"");
		GlobalDefinitions.Add("UE4_PROJECT_STEAMPRODUCTNAME=\"Longvinter\"");
		GlobalDefinitions.Add("UE4_PROJECT_STEAMGAMEDIR=\"Longvinter\"");
		GlobalDefinitions.Add("UE4_PROJECT_STEAMSHIPPINGID=1635450");
		ProjectDefinitions.Add("ONLINE_SUBSYSTEM_EOS_ENABLE_STEAM=1");
		

		
	}
}
