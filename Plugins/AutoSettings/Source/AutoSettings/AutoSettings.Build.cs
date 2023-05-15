// Copyright Sam Bonifacio. All Rights Reserved.

using UnrealBuildTool;

public class AutoSettings : ModuleRules
{
	public AutoSettings(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "UMG", "Slate", "SlateCore", "GameplayTags", "DeveloperSettings", "AutoSettingsCore" });

        PrivateDependencyModuleNames.AddRange(new string[] {  });

	}
}
