// Copyright Epic Games, Inc. All Rights Reserved.

using AutomationTool;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UnrealBuildTool;
using Tools.DotNETCommon;

using Microsoft.Win32;
using System.Diagnostics;

namespace SimpleUGC.Automation
{
	public class PackageUGC : BuildCommand
	{
		static public ProjectParams GetParams(BuildCommand Cmd, string ProjectFileName, out FileReference PluginFile)
		{
			string VersionString = Cmd.ParseParamValue("Version", "NOVERSION");

			// Get the plugin filename
			string PluginPath = Cmd.ParseParamValue("PluginPath");
			if (PluginPath == null)
			{
				throw new AutomationException("Missing -PluginPath=... argument");
			}

			// Check it exists
			PluginFile = new FileReference(PluginPath);
			if (!FileReference.Exists(PluginFile))
			{
				throw new AutomationException("Plugin '{0}' not found", PluginFile.FullName);
			}

			string ReleaseVersion = Cmd.ParseParamValue("BasedOnReleaseVersion", "Longvinter");

			FileReference ProjectFile = new FileReference(ProjectFileName);

			ProjectParams Params = new ProjectParams(
				RawProjectPath: ProjectFile,
				Command: Cmd,
				ClientTargetPlatforms: new List<TargetPlatformDescriptor>(){ new TargetPlatformDescriptor(UnrealTargetPlatform.Win64) },
				Build: false,
				Cook: true,
				Stage: true,
				Pak: true,
				Manifests: true,
				DLCIncludeEngineContent: true, // Need this to allow engine content that wasn't cooked in the base game to be included in the PAK file 
				BasedOnReleaseVersion: ReleaseVersion,
				DLCName: PluginFile.GetFileNameWithoutAnyExtensions(),

				RunAssetNativization: true
				);

			Params.ValidateAndLog();
			return Params;
		}

		public override void ExecuteBuild()
		{
			int WorkingCL = -1;
			FileReference PluginFile = null;
			string ProjectFileName = ParseParamValue("Project");
			if(ProjectFileName == null)
			{
				ProjectFileName = CombinePaths(CmdEnv.LocalRoot, "Longvinter", "Longvinter.uproject");
			}
			LogInformation(ProjectFileName);

			ProjectParams Params = GetParams(this, ProjectFileName, out PluginFile);

			// Check whether folder already exists so we know if we can delete it later
			string PlatformStageDir = Path.Combine(Params.StageDirectoryParam, "WindowsNoEditor");
			bool bPreExistingStageDir = Directory.Exists(PlatformStageDir);

			PluginDescriptor Plugin = PluginDescriptor.FromFile(PluginFile);

			FileReference ProjectFile = new FileReference(ProjectFileName);

			// Add Plugin to folders excluded for nativization in config file
			FileReference UserEditorIni = new FileReference(Path.Combine(Path.GetDirectoryName(ProjectFileName), "Config", "UserEditor.ini"));
			bool bPreExistingUserEditorIni = FileReference.Exists(UserEditorIni);
			if (!bPreExistingUserEditorIni)
			{
				// Expect this most of the time so we will create and clean up afterwards
				DirectoryReference.CreateDirectory(UserEditorIni.Directory);
				CommandUtils.WriteAllText(UserEditorIni.FullName, "");
			}

			const string ConfigSection = "BlueprintNativizationSettings";
			const string ConfigKey = "ExcludedFolderPaths";
			string ConfigValue = "/" + PluginFile.GetFileNameWithoutAnyExtensions() + "/";

			ConfigFile UserEditorConfig = new ConfigFile(UserEditorIni);
			ConfigFileSection BPNSection = UserEditorConfig.FindOrAddSection(ConfigSection);
			bool bUpdateConfigFile = !BPNSection.Lines.Exists(x => String.Equals(x.Key, ConfigKey, StringComparison.OrdinalIgnoreCase) && String.Equals(x.Value, ConfigValue, StringComparison.OrdinalIgnoreCase));
			if (bUpdateConfigFile)
			{
				BPNSection.Lines.Add(new ConfigLine(ConfigLineAction.Add, ConfigKey, ConfigValue));
				UserEditorConfig.Write(UserEditorIni);
			}

			Project.Cook(Params);
			if (!bPreExistingUserEditorIni)
			{
				FileReference.Delete(UserEditorIni);
			}

			Project.CopyBuildToStagingDirectory(Params);
			Project.Package(Params, WorkingCL);
			Project.Archive(Params);
			Project.Deploy(Params);

			// Get path to where the plugin was staged
			string StagedPluginDir = Path.Combine(PlatformStageDir, Path.GetFileNameWithoutExtension(ProjectFileName), PluginFile.Directory.MakeRelativeTo(ProjectFile.Directory));
			string ZipFile = Path.Combine(Params.StageDirectoryParam, PluginFile.GetFileNameWithoutAnyExtensions());
			CommandUtils.DeleteFile(ZipFile);
			System.IO.Compression.ZipFile.CreateFromDirectory(StagedPluginDir, ZipFile + ".zip");

			if (!bPreExistingStageDir)
			{
				CommandUtils.DeleteDirectory(PlatformStageDir);
			}
		}
	}
}
