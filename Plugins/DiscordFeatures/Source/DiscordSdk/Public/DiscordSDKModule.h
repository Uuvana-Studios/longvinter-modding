// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"


DISCORDSDK_API DECLARE_LOG_CATEGORY_EXTERN(LogDiscordSdk, Log, All);

class FDiscordSDKModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	bool HandleSettingsSaved();
};

