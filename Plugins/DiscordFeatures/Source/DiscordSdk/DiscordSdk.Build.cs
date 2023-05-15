// Copyright Pandores Marketplace 2021. All Rights Reserved.

using System;
using UnrealBuildTool;

using System.IO;
using System.Reflection;

public class DiscordSdk : ModuleRules
{
	/**
	 * Where the Discord Game SDK should be installed relative to this plugin
	 **/
	private const string SdkRelativeLocation = "ThirdParty/DiscordSdk/lib/";

	public DiscordSdk(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));

		PrivateDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" , "Projects" });

		if (Target.Type == TargetType.Editor)
		{
			PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore", "UnrealEd", "PropertyEditor" });
		}
 
		SdkRootDir = Path.Combine(PluginDirectory, SdkRelativeLocation);
		
		switch (Target.Platform.ToString())
		{
		case "Win64": AddDynamicLibraryChecked("x86_64/discord_game_sdk.dll");	 break;
#if !UE_5_0_OR_LATER
		case "Win32": AddDynamicLibraryChecked(   "x86/discord_game_sdk.dll");   break;
#endif
		case "Mac"  : AddDynamicLibraryChecked("x86_64/discord_game_sdk.dylib"); break;
		case "Linux": AddDynamicLibraryChecked("x86_64/discord_game_sdk.so");	 break;
		
		default: DefineDiscordSdkocation("INVALID_PLATFORM"); break;
		}
	}

	private void AddDynamicLibraryChecked(string Path)
	{
		string FullPath = SdkRootDir + Path;
		
		DefineDiscordSdkocation(SdkRelativeLocation + Path);

		RuntimeDependencies.Add(FullPath);

		if (!File.Exists(FullPath))
		{
			PrintBoxed(new string[]
			{
				"Failed to find Discord SDK DLL.",
				"Please, download the Discord SDK to",
				"\"DiscordFeatures/ThirdParty/DiscordSdk\"", 
				"to be able to use Discord Game SDK Features."
			});
		}
	}

	private void DefineDiscordSdkocation(string Location)
    {
		PublicDefinitions.Add("DISCORD_SDK_DLL=\"" + Location + "\"");
	}

	private void PrintBoxed(string[] Lines)
    {
		int Length = 0;
		for (int i = 0; i < Lines.Length; ++i)
        {
			if (Length < Lines[i].Length)
            {
				Length = Lines[i].Length;
            }
        }

		string	  OutPad	= "   ";
		string	  Output	= "\n";
		const int Padding	= 4;

		Output += OutPad + (char)9556;
		for (int i = 0; i < Length + 2 * Padding; ++i)
        {
			Output += (char)9552;
		}
		Output += (char)9559 + "\n";

		for (int i = 0; i < Lines.Length; ++i)
        {
			const int BoxSide = 9553;
			int LineLength = Lines[i].Length;
			Output += OutPad + (char)BoxSide + 
				Lines[i].PadLeft(LineLength + Padding).PadRight(LineLength + 2* Padding + Length - LineLength) 
				+ (char)BoxSide + "\n";
        }

		Output += OutPad + (char)9562;
		for (int i = 0; i < Length + 2 * Padding; ++i)
		{
			Output += (char)9552;
		}
		Output += (char)9565 + "\n";

		Console.WriteLine(Output);
    }

	private string SdkRootDir;
}
