// Copyright (c) 2022 Semyon Gritsenko

#pragma once

#include "Modules/ModuleManager.h"

class FAsyncBlueprintsExtensionModule : public IModuleInterface
{
public:
	void StartupModule() override;

	void ShutdownModule() override;
};
