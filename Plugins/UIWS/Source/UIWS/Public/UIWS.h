// Copyright 2018 Elliot Gray. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

//DECLARE_STATS_GROUP(TEXT("UIWS"), STATGROUP_UIWS, STATCAT_Advanced);

class FUIWSModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
