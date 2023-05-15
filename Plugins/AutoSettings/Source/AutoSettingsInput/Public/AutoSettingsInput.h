// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FAutoSettingsInputModule : public IModuleInterface
{
public:

	// Static get module
	static inline FAutoSettingsInputModule* Get()
	{
		return FModuleManager::LoadModulePtr< FAutoSettingsInputModule >("AutoSettingsInput");
	}

protected:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

};
