// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "AutoSettingsConfig.generated.h"

// Plugin configuration page in Project Settings
UCLASS(config=Game, defaultconfig)
class AUTOSETTINGS_API UAutoSettingsConfig : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	UAutoSettingsConfig();

	// Config ini file (without extension) that settings are saved to.
	// Requires engine restart for changes to take effect.
	UPROPERTY(Config, EditAnywhere, Category = Settings)
	FString SettingsIniName = "Settings";

	// Config section that settings are saved to within the config ini file
	UPROPERTY(Config, EditAnywhere, Category = Settings)
	FString SettingsSectionName = "Settings";

protected:

	virtual void PostInitProperties() override;

#if WITH_EDITOR
	virtual FText GetSectionText() const override;
	virtual FText GetSectionDescription() const override;
#endif

private:

};
