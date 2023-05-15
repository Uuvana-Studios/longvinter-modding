// Copyright (C) 2020 Alan Renato Bunese - All Rights Reserved.

#include "BlueprintableOnlineBeacons.h"

#if WITH_EDITOR

#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "ISettingsContainer.h"
#include "OnlineBeaconsSettings.h"

#endif

#define LOCTEXT_NAMESPACE "FBlueprintableOnlineBeaconsModule"

// Define the Blueprintable Online Beacons Log.
DEFINE_LOG_CATEGORY(LogBlueprintableOnlineBeacons);

// Called by the Engine when the Module Starts up.
void FBlueprintableOnlineBeaconsModule::StartupModule() {
#if WITH_EDITOR
	// Get the Settings Module.
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	// If our Module is valid.
	if (SettingsModule) {
		// Create the Online Beacons Settings Section.
		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Project", "Plugins", "BlueprintableOnlineBeacons", LOCTEXT("BlueprintableOnlineBeacons", "Blueprintable Online Beacons"), LOCTEXT("BlueprintableOnlineBeaconsDescription", "Configure anything related to Online Beacons, without going into INI files."), GetMutableDefault<UOnlineBeaconsSettings>());

		// If creation was a success, bind modified to the Save Settings function.
		if (SettingsSection.IsValid())
			SettingsSection->OnModified().BindUObject(GetMutableDefault<UOnlineBeaconsSettings>(), &UOnlineBeaconsSettings::SaveBeaconSettings);
	}
#endif
}

// Called by the Engine when the Module Shutsdown.
void FBlueprintableOnlineBeaconsModule::ShutdownModule() {
#if WITH_EDITOR
	// Get the Settings Module.
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	
	// Check for null, unregister settings.
	if (SettingsModule)
		SettingsModule->UnregisterSettings("Project", "Plugins", "BlueprintableOnlineBeacons");
#endif
}

// If this module support dynamic reloading. (For Easy Code Compilation and Reloading)
bool FBlueprintableOnlineBeaconsModule::SupportsDynamicReloading() {
	return true;
}

// If this module is used as a game module.
bool FBlueprintableOnlineBeaconsModule::IsGameModule() const {
	return true;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBlueprintableOnlineBeaconsModule, BlueprintableOnlineBeacons)