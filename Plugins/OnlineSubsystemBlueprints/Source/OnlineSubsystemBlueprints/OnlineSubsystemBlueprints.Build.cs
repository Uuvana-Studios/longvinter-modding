// Copyright June Rhodes. All Rights Reserved.

using UnrealBuildTool;
using System.IO;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
#if UE_5_0_OR_LATER
using EpicGames.Core;
#else
using Tools.DotNETCommon;
#endif

public class OnlineSubsystemBlueprints : ModuleRules
{
#if !UE_4_26_OR_LATER
    private void ConditionalAddModuleDirectory(DirectoryReference TargetDirectory)
    {
        FieldInfo Field = this.GetType().GetField("DirectoriesForModuleSubClasses", BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance);
        if (Field == null)
        {
            throw new Exception("Expected to find DirectoriesForModuleSubClasses field on ModuleRules class.");
        }
        var Dict = (Dictionary<Type, DirectoryReference>)Field.GetValue(this);
        if (Dict == null)
        {
            Dict = new Dictionary<Type, DirectoryReference>();
        }
        Dict.Add(typeof(OnlineSubsystemBlueprints), TargetDirectory);
        Field.SetValue(this, Dict);
    }
#endif

    public OnlineSubsystemBlueprints(ReadOnlyTargetRules Target) : base(Target)
    {
        DefaultBuildSettings = BuildSettingsVersion.V2;
        bUsePrecompiled = false;

#if UE_5_0_OR_LATER
        this.PrivateDefinitions.Add("UE_5_0_OR_LATER=1");
#endif
#if UE_4_27_OR_LATER
        this.PrivateDefinitions.Add("UE_4_27_OR_LATER=1");
#endif
#if UE_4_26_OR_LATER
        this.PrivateDefinitions.Add("UE_4_26_OR_LATER=1");
#endif
#if UE_4_25_OR_LATER
        this.PrivateDefinitions.Add("UE_4_25_OR_LATER=1");
#endif
#if UE_4_24_OR_LATER
        this.PrivateDefinitions.Add("UE_4_24_OR_LATER=1");
#endif

#if UE_5_0_OR_LATER && !UE_5_1_OR_LATER
        Console.WriteLine("Online Subsystem Blueprints: Targeting Unreal Engine 5.0.");
        this.ConditionalAddModuleDirectory(new DirectoryReference(Path.Combine(this.ModuleDirectory, "..", "OnlineSubsystemBlueprints_5.0")));
#elif UE_4_27_OR_LATER && !UE_5_0_OR_LATER
        Console.WriteLine("Online Subsystem Blueprints: Targeting Unreal Engine 4.27.");
        this.ConditionalAddModuleDirectory(new DirectoryReference(Path.Combine(this.ModuleDirectory, "..", "OnlineSubsystemBlueprints_4.27")));
#elif UE_4_26_OR_LATER && !UE_4_27_OR_LATER
        Console.WriteLine("Online Subsystem Blueprints: Targeting Unreal Engine 4.26.");
        this.ConditionalAddModuleDirectory(new DirectoryReference(Path.Combine(this.ModuleDirectory, "..", "OnlineSubsystemBlueprints_4.26")));
#elif UE_4_25_OR_LATER && !UE_4_26_OR_LATER
        Console.WriteLine("Online Subsystem Blueprints: Targeting Unreal Engine 4.25.");
        this.ConditionalAddModuleDirectory(new DirectoryReference(Path.Combine(this.ModuleDirectory, "..", "OnlineSubsystemBlueprints_4.25")));
#elif UE_4_24_OR_LATER && !UE_4_25_OR_LATER
        Console.WriteLine("Online Subsystem Blueprints: Targeting Unreal Engine 4.24.");
        this.ConditionalAddModuleDirectory(new DirectoryReference(Path.Combine(this.ModuleDirectory, "..", "OnlineSubsystemBlueprints_4.24")));
#else
        throw new Exception("Online Subsystem Blueprints is not compatible with this version of Unreal Engine. You can add compatibility for this engine version by modifying OnlineSubsystemBlueprints.Build.cs and running \"powershell .\\Generator\\Generate.ps1 -EngineVersion 4.__\".");
#endif

        /* PRECOMPILED REMOVE BEGIN */
        if (!bUsePrecompiled)
        {
            if (Environment.GetEnvironmentVariable("BUILDING_FOR_REDISTRIBUTION") == "true")
            {
                bTreatAsEngineModule = true;
                bPrecompile = true;

                // Force the module to be treated as an engine module for UHT, to ensure UPROPERTY compliance.
#if UE_5_0_OR_LATER
                object ContextObj = this.GetType().GetProperty("Context", BindingFlags.Instance | BindingFlags.NonPublic).GetValue(this);
#else
                object ContextObj = this.GetType().GetField("Context", BindingFlags.Instance | BindingFlags.NonPublic).GetValue(this);
#endif
                ContextObj.GetType().GetField("bClassifyAsGameModuleForUHT", BindingFlags.Instance | BindingFlags.Public).SetValue(ContextObj, false);
            }

            PublicDependencyModuleNames.AddRange(
                new string[]
                {
                    "Core",
                    "OnlineSubsystemUtils",
                }
            );

            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "CoreUObject",
                    "Engine",
                    "OnlineSubsystem",
#if UE_4_25_OR_LATER
                    "VoiceChat",
#endif
#if UE_5_0_OR_LATER
                    "CoreOnline",
#endif
                }
            );
        }
        /* PRECOMPILED REMOVE END */
    }
}