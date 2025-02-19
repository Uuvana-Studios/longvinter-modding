// Copyright Qibo Pang 2022. All Rights Reserved.

#include "ImageCacheModule.h"
#include "ImageCacheSettings.h"

#if WITH_EDITOR
#include "ISettingsModule.h"	// Settings
#include "ISettingsSection.h"	// Settings
#include "ISettingsContainer.h"	// Settings

const char* const ConfigCantainer = "Project";
const char* const ConfigSection = "Image Cache Settings";

#endif

#define LOCTEXT_NAMESPACE "FImageCacheModule"

void FImageCacheModule::StartupModule()
{
#if WITH_EDITOR
	//-- register settings
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule) {

		FName ContainerName(ConfigCantainer);
		FName SectionName(ConfigSection);

		// create new category for MVM
		ISettingsContainerPtr SettingsContainer = SettingsModule->GetContainer(ContainerName);

		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings(ContainerName,
			"Project",
			SectionName,
			FText::FromString(TEXT("Image Cache")),
			FText::FromString(TEXT("Image Cache Settings")),
			GetMutableDefault<UImageCacheSettings>()
		);

	}// end of if
#endif
}

void FImageCacheModule::ShutdownModule()
{
	if (!UObjectInitialized())
		return;

#if WITH_EDITOR
	//-- unregister settings
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule) {
		SettingsModule->UnregisterSettings(
			ConfigCantainer,
			"Project",
			ConfigSection
		);
	}
#endif
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FImageCacheModule, ImageCache)
DEFINE_LOG_CATEGORY(ImageCacheLog);