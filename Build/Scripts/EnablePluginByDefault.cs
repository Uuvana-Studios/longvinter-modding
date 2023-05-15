// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using System.Collections.Generic;
using System.IO;
using System.Threading;
using System.Reflection;
using AutomationTool;
using UnrealBuildTool;
using Tools.DotNETCommon;

using System.Linq;
using System.Text;
using Microsoft.Win32;
using System.Diagnostics;

namespace SimpleUGC.Automation
{
	[Help("Sets the EnabledByDefault flag for a plugin to true")]
	public class EnablePluginByDefault : BuildCommand
	{
		public override void ExecuteBuild()
		{
			string FileName = ParseParamValue("FileName", null);
			if(FileName == null)
			{
				throw new AutomationException("Missing -FileName=... argument");
			}

			CommandUtils.SetFileAttributes(FileName, ReadOnly: false);
			
			PluginDescriptor Plugin = PluginDescriptor.FromFile(new FileReference(FileName));
			Plugin.bEnabledByDefault = true;
			Plugin.Save(FileName);
		}
	}
}
