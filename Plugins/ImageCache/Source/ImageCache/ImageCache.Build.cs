// Copyright Qibo Pang 2022. All Rights Reserved.

using UnrealBuildTool;
using System;
using System.IO;
using System.Text;
using Microsoft.Win32;


public class ImageCache : ModuleRules
{
	public ImageCache(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "HTTP",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "Slate",
                "SlateCore",
                "RenderCore",
                "RHI",
                "ImageWriteQueue",
				"Projects",
				"Json"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

        if (Target.Type != TargetType.Server)
        {
            PrivateIncludePathModuleNames.AddRange(
                new string[] {
                    "SlateRHIRenderer",
                }
            );

            DynamicallyLoadedModuleNames.AddRange(
                new string[] {
                    "ImageWrapper",
                    "SlateRHIRenderer",
                }
            );
        }

		if (Target.Type == TargetRules.TargetType.Editor)
		{
			PrivateDependencyModuleNames.AddRange(
			new string[]
			{
			"Settings"
			}
			);
		}
	}
}
