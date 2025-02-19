// Copyright (c) 2022 Semyon Gritsenko

using System;
using System.IO;
using UnrealBuildTool;

public class AsyncBlueprintsExtension : ModuleRules
{
	public AsyncBlueprintsExtension(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange
		(
			new string[]
			{
				Path.Combine(ModuleDirectory, "Public")
			}
		);

		PrivateIncludePaths.AddRange
		(
			new string[]
			{
				Path.Combine(ModuleDirectory, "Private")
			}
		);

		PublicDependencyModuleNames.AddRange
		(
			new string[]
			{
				"Core"
			}
		);

		PrivateDependencyModuleNames.AddRange
		(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore"
			}
		);
	}
}
