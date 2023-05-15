// Copyright Sam Bonifacio. All Rights Reserved.

#include "Misc/AutoSettingsConfig.h"

#define LOCTEXT_NAMESPACE "FAutoSettingsModule"

UAutoSettingsConfig::UAutoSettingsConfig()
{
	CategoryName = "Plugins";
}

void UAutoSettingsConfig::PostInitProperties()
{
	Super::PostInitProperties();
	
	// Migrate deprecated properties
	// None for now

}

#if WITH_EDITOR
FText UAutoSettingsConfig::GetSectionText() const
{
	return LOCTEXT("AutoSettingsName", "Auto Settings");
}

FText UAutoSettingsConfig::GetSectionDescription() const
{
	return LOCTEXT("AutoSettingsDescription", "Configure the Auto Settings plugin.");
}
#endif

#undef LOCTEXT_NAMESPACE
