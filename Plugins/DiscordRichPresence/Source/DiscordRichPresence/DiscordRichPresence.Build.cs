// Discord Rich Presence Plugin - (c) Jonathan Verbeek 2019

using UnrealBuildTool;
using System.IO;

public class DiscordRichPresence : ModuleRules
{
	public DiscordRichPresence(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        // Compose the paths for including the nessecary files
        string BaseDirectory = System.IO.Path.GetFullPath(System.IO.Path.Combine(ModuleDirectory, "..", ".."));
        string DiscordDirectory = System.IO.Path.Combine(BaseDirectory, "Source", "ThirdParty", "x86_64");

        PublicIncludePaths.AddRange(
            new string[] {
				// ... add public include paths required here ...
                Path.Combine(ModuleDirectory, "Public"),
                Path.Combine(ModuleDirectory, "Public", "Discord")
			}
		);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
                Path.Combine(ModuleDirectory, "Private"),
                Path.Combine(ModuleDirectory, "Private", "Discord")
    }
		);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
		);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "Projects"
				// ... add private dependencies that you statically link with here ...	
			}
		);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
		);

        // Only compile in the Discord libraries on Win64
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicDefinitions.Add("WITH_DISCORD=1");
            PublicLibraryPaths.Add(DiscordDirectory);
            PublicDelayLoadDLLs.Add("discord_game_sdk.dll");
            PublicAdditionalLibraries.Add(System.IO.Path.Combine(DiscordDirectory, "discord_game_sdk.dll.lib"));
            RuntimeDependencies.Add("$(TargetOutputDir)/discord_game_sdk.dll", Path.Combine(DiscordDirectory, "discord_game_sdk.dll"));
        }
	}
}
