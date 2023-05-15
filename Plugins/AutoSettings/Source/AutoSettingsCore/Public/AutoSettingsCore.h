// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FAutoSettingsCoreModule : public IModuleInterface
{
public:

protected:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

};
