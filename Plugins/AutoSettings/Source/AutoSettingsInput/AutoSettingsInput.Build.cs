// Copyright Sam Bonifacio. All Rights Reserved.

using UnrealBuildTool;

public class AutoSettingsInput : ModuleRules
{
	public AutoSettingsInput(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "UMG", "Slate", "SlateCore", "GameplayTags", "DeveloperSettings", "AutoSettingsCore" });

        PrivateDependencyModuleNames.AddRange(new string[] {  });

	}
}
