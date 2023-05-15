// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "DiscordSDKModule.h"
#include "DiscordSDK.h"
#include "DiscordSettings.h"

#if WITH_EDITOR
	#include "ISettingsModule.h"
	#include "ISettingsSection.h"
#endif

DEFINE_LOG_CATEGORY(LogDiscordSdk);

#define LOCTEXT_NAMESPACE "FDiscordSDKModule"

void FDiscordSDKModule::StartupModule()
{
#if WITH_EDITOR
	// register settings
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	if (SettingsModule)
	{
		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Project", "Plugins", "Discord Features",
			LOCTEXT("DiscordFeaturesName", "Discord Features"),
			LOCTEXT("DiscordFeaturesSettingsDescription", "Configure the Discord Features plug-in."),
			GetMutableDefault<UDiscordSettings>()
		);

		if (SettingsSection.IsValid())
		{
			SettingsSection->OnModified().BindRaw(this, &FDiscordSDKModule::HandleSettingsSaved);
		}
	}
#endif // WITH_EDITOR

#if WITH_EDITOR
	UDiscordSettings::TestSdkPaths();
#endif // WITH_EDITOR

	FDiscordSdk::Get()->SettingsUpdated();
}

void FDiscordSDKModule::ShutdownModule()
{
#if WITH_EDITOR
	// Unregister settings
	ISettingsModule* const SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	if (SettingsModule)
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "Discord Features");
	}
#endif
	FDiscordSdk::Get()->Free();
}

bool FDiscordSDKModule::HandleSettingsSaved()
{
#if WITH_EDITOR
	UDiscordSettings::TestSdkPaths();

	FDiscordSdk::Get()->SettingsUpdated();
#endif

	return true;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDiscordSDKModule, DiscordSdk)
