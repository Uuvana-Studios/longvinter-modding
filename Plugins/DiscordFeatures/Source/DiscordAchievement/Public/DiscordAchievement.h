// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FDiscordAchievementModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

