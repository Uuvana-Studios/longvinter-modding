// Copyright Qibo Pang 2022. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "ImageCache.h"

class FImageCacheModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

DECLARE_LOG_CATEGORY_EXTERN(ImageCacheLog, Log, All);